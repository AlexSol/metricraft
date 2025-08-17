#pragma once
#include <string>
#include <map>
#include <vector>
#include <sstream>

struct Metric {
    std::string name;
    std::string type;     // counter|gauge|histogram (MVP: counter|gauge)
    double      value{};
    std::map<std::string,std::string> labels;
};

struct MetricsBatch {
    std::vector<Metric> items;
};

inline std::string to_prom(const MetricsBatch& b) {
    std::ostringstream os;
    for (const auto& m : b.items) {
        os << "# TYPE " << m.name << ' ' << m.type << "\n";
        os << m.name;
        if (!m.labels.empty()) {
            os << '{';
            bool first=true;
            for (auto& [k,v]: m.labels) { if(!first) os<<','; first=false; os<<k<<"=\""<<v<<"\""; }
            os << '}';
        }
        os << ' ' << m.value << "\n";
    }
    return os.str();
}
