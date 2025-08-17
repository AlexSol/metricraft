#pragma once
#include <vector>
#include <random>
#include <mutex>
#include <queue>
#include <chrono>
#include <condition_variable>
#include <atomic>

#include "plugin.hpp"
#include "manifest.hpp"

struct Task {
    Manifest manifest;
    std::unique_ptr<IRunner> runner;
    std::chrono::steady_clock::time_point next;
};

class Scheduler {
    std::mutex mtx_;
    std::condition_variable cv_;
    std::vector<Task> tasks_;
    std::atomic<bool> stop_{false};
public:
    void add(Task task) {
        task.next = std::chrono::steady_clock::now() + task.manifest.interval;
        std::scoped_lock lk(mtx_);
        tasks_.push_back(std::move(task));
        cv_.notify_all();
    }
    template<typename Fn> void run(Fn on_metrics) {
        std::mt19937_64 rng{std::random_device{}()};
        while(!stop_) {
            auto now = std::chrono::steady_clock::now();
            std::unique_lock lk(mtx_);
            auto next_tp = now + std::chrono::seconds(3600);

            for (auto& task: tasks_) {
                if (task.next <= now) {
                    lk.unlock();
                    // jitter
                    auto j = task.manifest.jitter.count()
                           ? std::uniform_int_distribution<long long>(
                                 0,task.manifest.jitter.count())(rng)
                           : 0;
                    auto ctx = nlohmann::json::object(); // TODO: заповнити контекст
                   
                    if (auto b = task.runner->collect(ctx)) {
                        on_metrics(task.manifest.name,*b);
                    }
                   
                    task.next = std::chrono::steady_clock::now() + task.manifest.interval + std::chrono::milliseconds(j);
                    lk.lock();
                }

                next_tp = std::min(next_tp, task.next);
            }

            cv_.wait_until(lk, next_tp, [&]{return stop_.load();});
        }
    }
    void shutdown(){ stop_ = true; cv_.notify_all(); }
};
