#pragma once

#include <chrono>
#include <optional>

#include "advrf_middleware_core/config/robot_config.hpp"

namespace advrf::middleware::config {

struct RuntimeConfig {
    uint32_t rate_publishers = 1000;
    uint32_t rate_subscribers = 100;
    uint32_t rate_service = 10;
};

inline std::chrono::microseconds period_from_rate(uint32_t hz) {
    if (hz == 0 || hz > 1'000'000)
        throw ConfigError("rate: must be between 1 and 1000000 Hz");
    return std::chrono::microseconds(1'000'000 / hz);
}
inline void validate(const RuntimeConfig& cfg) {
    const auto check = [](uint32_t hz, const char* field) {
        if (hz == 0 || hz > 1'000'000)
            throw ConfigError(std::string("runtime.") + field + ": must be between 1 and 1000000 Hz");
    };
    check(cfg.rate_publishers, "publishers_hz");
    check(cfg.rate_subscribers, "subscribers_hz");
    check(cfg.rate_service, "service_hz");
}
inline RuntimeConfig parse_runtime_config(const YAML::Node& root) {
    const auto node = section(root, "runtime");
    check_keys(node, {"publishers_hz", "subscribers_hz", "service_hz"}, "runtime");
    RuntimeConfig cfg;
    cfg.rate_publishers = unsigned_value(node, "publishers_hz", cfg.rate_publishers, "runtime");
    cfg.rate_subscribers = unsigned_value(node, "subscribers_hz", cfg.rate_subscribers, "runtime");
    cfg.rate_service = unsigned_value(node, "service_hz", cfg.rate_service, "runtime");
    return cfg;
}
struct CommonConfig {
    RobotConfig robot;
    RuntimeConfig runtime;
};
inline CommonConfig parse_common_config(const ConfigFile& file) {
    return {parse_runtime_robot_config(file.root), parse_runtime_config(file.root)};
}
inline void validate(const CommonConfig& cfg) { validate(cfg.robot); validate(cfg.runtime); }

struct RuntimeOverrides {
    std::optional<uint32_t> publishers, subscribers, service;
    void apply(RuntimeConfig& cfg) const {
        if (publishers) cfg.rate_publishers = *publishers;
        if (subscribers) cfg.rate_subscribers = *subscribers;
        if (service) cfg.rate_service = *service;
    }
};

// Shared CLI flags; plugins handle their own transport-specific flags.
struct CommonOptions {
    std::filesystem::path config_path;
    RuntimeOverrides runtime;
    bool help = false;

    bool consume(const std::string& flag, const std::string& text) {
        if (flag == "--config") { config_path = text; return true; }
        std::optional<uint32_t>* target = nullptr;
        if (flag == "--rate-publishers" || flag == "--rate_publishers") target = &runtime.publishers;
        else if (flag == "--rate-subscribers" || flag == "--rate_subscribers") target = &runtime.subscribers;
        else if (flag == "--rate-service" || flag == "--rate_service") target = &runtime.service;
        if (!target) return false;
        YAML::Node node; node["rate"] = text;
        *target = unsigned_value(node, "rate", 0, flag);
        return true;
    }
};

inline std::string argument_value(int& index, int argc, char** argv) {
    if (index + 1 >= argc || std::string(argv[index + 1]).rfind("--", 0) == 0)
        throw ConfigError("Missing value after " + std::string(argv[index]));
    return argv[++index];
}
inline CommonOptions parse_common_options(int argc, char** argv) {
    CommonOptions options;
    for (int index = 1; index < argc; ++index) {
        const std::string flag = argv[index];
        if (flag == "--help" || flag == "-h") { options.help = true; continue; }
        const auto text = argument_value(index, argc, argv);
        if (!options.consume(flag, text)) throw ConfigError("Unknown argument: " + flag);
    }
    return options;
}

inline const char* common_options_help() {
    return "  --config <PATH>           Middleware YAML configuration\n"
           "  --rate-publishers <Hz>   Override publisher frequency\n"
           "  --rate-subscribers <Hz>  Override subscriber frequency\n"
           "  --rate-service <Hz>      Override service frequency\n";
}

} // namespace advrf::middleware::config
