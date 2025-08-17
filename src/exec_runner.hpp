#pragma once
#include "plugin.hpp"
#include "manifest.hpp"
#include <boost/process.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/readable_pipe.hpp>
#include <boost/system/error_code.hpp>
#include <boost/asio.hpp>

#include <future>
#include <thread>
#include <chrono>
#include <atomic>
// #include <boost/process.hpp>
// або якщо потрібні конкретні компоненти:
// #include <boost/process/child.hpp>
// #include <boost/process/io.hpp>
// #include <boost/process/pipe.hpp>

#include <iostream>

class ExecRunner : public IRunner {
    Manifest m_;
public:
    explicit ExecRunner(Manifest m): m_(std::move(m)) {}
    
    std::optional<MetricsBatch> collect(const nlohmann::json& ctx) override {
        namespace bp = boost::process;
        
        namespace proc   = boost::process;
        namespace asio = boost::asio;
        // using boost::process::process;

        
        try {
            asio::io_context ctx;
            asio::readable_pipe p{ctx};

            const auto exe = proc::environment::find_executable("gcc");

            proc::process c{ctx, exe, {"--version"}, proc::process_stdio{nullptr, p}};

            std::string line;
            boost::system::error_code ec;

            auto sz = asio::read(p, asio::dynamic_buffer(line), ec);
            assert(ec == asio::error::eof);

            std::cout << "Gcc version: '"  << line << "'" << std::endl;

            c.wait();

            // Create pipes
            // bp::pipe stdin_pipe{io_context};
            // bp::pipe stdout_pipe{io_context};

                // std::string output;
                // boost::process::ipstream pipe_stream; // Stream to capture the output

                // // Execute the "ls" command and redirect the output to pipe_stream
                // boost::process::child c("ls", boost::process::std_out > pipe_stream);

                // // Read the command output line by line
                // std::string line;
                // while (std::getline(pipe_stream, line)) {
                //     std::cout << line << std::endl;
                // }

                // c.wait(); // Wait for the process to finish

                    // boost::process::ipstream out;
                    // boost::process::child c("ls", boost::process::std_out > out);
                    // std::string line; while (std::getline(out, line)) {
                    //     std::cout << line << std::endl;
                    // }

            // boost::process::ipstream out;
            // boost::process::opstream in;
            
            // // Запускаємо процес з обробкою помилок
            // boost::process::child c(m_.entry, 
            //     bp::std_out > out, 
            //     bp::std_in < in, 
            //     bp::std_err > bp::null);
            
            // if (!c.valid()) {
            //     return std::nullopt;
            // }

            // // Передаємо ctx у stdin з обробкою помилок
            // try {
            //     in << ctx.dump() << std::endl;
            //     in.pipe().close();
            // } catch (const std::exception&) {
            //     c.terminate();
            //     return std::nullopt;
            // }

            // // Читаємо stdout з таймаутом
            // std::promise<std::string> promise;
            // auto future = promise.get_future();
            // std::atomic<bool> should_stop{false};
            
            // std::thread reader_thread([&]() {
            //     try {
            //         std::string result, line;
            //         while (std::getline(out, line) && !should_stop.load()) {
            //             result += line + "\n";
            //         }
            //         if (!should_stop.load()) {
            //             promise.set_value(std::move(result));
            //         }
            //     } catch (const std::exception&) {
            //         if (!should_stop.load()) {
            //             promise.set_exception(std::current_exception());
            //         }
            //     }
            // });

            // // Чекаємо на завершення з таймаутом
            // auto timeout_duration = std::chrono::milliseconds(m_.timeout);
            // if (future.wait_for(timeout_duration) != std::future_status::ready) {
            //     should_stop.store(true);
            //     c.terminate();
                
            //     // Даємо трохи часу на graceful shutdown
            //     if (reader_thread.joinable()) {
            //         reader_thread.join();
            //     }
                
            //     return std::nullopt;
            // }

            // if (reader_thread.joinable()) {
            //     reader_thread.join();
            // }

            // // Чекаємо завершення процесу
            // c.wait();
            
            // // Перевіряємо код виходу
            // if (c.exit_code() != 0) {
            //     return std::nullopt;
            // }

            // std::string output;
            // try {
            //     output = future.get();
            // } catch (const std::exception&) {
            //     return std::nullopt;
            // }

            // // Парсимо JSON з обробкою помилок
            // nlohmann::json json_result;
            // try {
            //     json_result = nlohmann::json::parse(output);
            // } catch (const nlohmann::json::parse_error&) {
            //     return std::nullopt;
            // }

            // if (!json_result.contains("metrics") || !json_result["metrics"].is_array()) {
            //     return std::nullopt;
            // }

            MetricsBatch batch;
            // for (const auto& metric_json : json_result["metrics"]) {
            //     if (!metric_json.is_object()) {
            //         continue; // Пропускаємо невалідні метрики
            //     }
                
            //     Metric metric;
                
            //     // Валідація обов'язкових полів
            //     if (!metric_json.contains("name") || !metric_json["name"].is_string()) {
            //         continue;
            //     }
            //     metric.name = metric_json["name"].get<std::string>();
                
            //     if (metric.name.empty()) {
            //         continue; // Пропускаємо метрики з порожніми іменами
            //     }
                
            //     // Перевіряємо валідність імені метрики (базова перевірка)
            //     if (!isValidMetricName(metric.name)) {
            //         continue;
            //     }
                
            //     metric.type = metric_json.value("type", "gauge");
                
            //     if (!metric_json.contains("value")) {
            //         continue;
            //     }
                
            //     // Обробка різних типів значень
            //     if (metric_json["value"].is_number()) {
            //         metric.value = metric_json["value"].get<double>();
            //     } else if (metric_json["value"].is_string()) {
            //         try {
            //             metric.value = std::stod(metric_json["value"].get<std::string>());
            //         } catch (const std::exception&) {
            //             continue; // Пропускаємо метрики з невалідними значеннями
            //         }
            //     } else {
            //         continue;
            //     }
                
            //     // Обробка лейблів
            //     if (metric_json.contains("labels") && metric_json["labels"].is_object()) {
            //         for (const auto& [key, value] : metric_json["labels"].items()) {
            //             if (value.is_string()) {
            //                 metric.labels[key] = value.get<std::string>();
            //             } else {
            //                 metric.labels[key] = value.dump();
            //             }
            //         }
            //     }
                
            //     batch.items.push_back(std::move(metric));
            // }
            
            return batch;
            
        } catch (const std::exception&) {
            return std::nullopt;
        }
    }
    
private:
    // Базова валідація імені метрики
    bool isValidMetricName(const std::string& name) const {
        if (name.empty()) return false;
        
        // Перевіряємо перший символ (має бути літера або підкреслення)
        char first = name[0];
        if (!std::isalpha(first) && first != '_') {
            return false;
        }
        
        // Перевіряємо решту символів (літери, цифри, підкреслення)
        for (size_t i = 1; i < name.length(); ++i) {
            char c = name[i];
            if (!std::isalnum(c) && c != '_') {
                return false;
            }
        }
        
        return true;
    }
};