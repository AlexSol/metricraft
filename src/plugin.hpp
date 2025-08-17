#pragma once
#include <string>
#include <optional>
#include <nlohmann/json.hpp>
#include "metrics.hpp"

struct IRunner {
    virtual ~IRunner() = default;
    virtual std::optional<MetricsBatch> collect(const nlohmann::json& ctx) = 0;
};
