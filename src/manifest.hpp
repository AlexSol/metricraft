#pragma once
#include <yaml-cpp/yaml.h>
#include <string>
#include <chrono>

struct Manifest {
    std::string name;
    std::string runtime;
    std::string entry;
    std::chrono::milliseconds interval{10000};
    std::chrono::milliseconds jitter{0};
    std::chrono::milliseconds timeout{5000};

    // спрощено: ресурси/капи з маніфеста парсимо за потреби
    static Manifest load(const std::string& path) {
        auto y = YAML::LoadFile(path);

        Manifest m;
        m.name    = y["name"].as<std::string>();
        m.runtime = y["runtime"].as<std::string>();
        m.entry   = y["entry"].as<std::string>();
        
        auto parse = [](std::string s)->std::chrono::milliseconds{
            if(s.ends_with("ms")) return std::chrono::milliseconds(std::stoll(s));
            if(s.ends_with("s"))  return std::chrono::seconds(std::stoll(s));
            return std::chrono::seconds(std::stoll(s));
        };
        
        if (auto s=y["schedule"]; s) {
            if (s["interval"]) m.interval = parse(s["interval"].as<std::string>());
            if (s["jitter"])   m.jitter   = parse(s["jitter"].as<std::string>());
        }

        if (y["timeout"]) m.timeout = parse(y["timeout"].as<std::string>());

        return m;
    }
};
