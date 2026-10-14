#pragma once

#include <advrf_middleware_core/config/runtime_config.hpp>

#include "advrf_dds_common/qos/reader_policy.hpp"

namespace advrf::dds_common {

// The same portable endpoint policy is used for state writers and command readers.
using EndpointPolicy = ReaderPolicy;
struct DdsConfig {
    uint32_t domain_id = 0;
    bool declare_to_ros = false;
    EndpointPolicy state;
    EndpointPolicy command;
};

inline EndpointPolicy parse_endpoint_policy(const YAML::Node& node, const std::string& path) {
    using namespace advrf::middleware::config;
    check_keys(node, {"reliability", "durability", "history", "depth"}, path);
    EndpointPolicy policy;
    try {
        policy = ReaderPolicyBuilder()
            .set_reliability(value<std::string>(node, "reliability", "best_effort", path))
            .set_durability(value<std::string>(node, "durability", "volatile", path))
            .set_history_depth(unsigned_value(node, "depth", 1, path))
            .build();
    } catch (const std::invalid_argument& e) { throw ConfigError(path + ": " + e.what()); }
    if (value<std::string>(node, "history", "keep_last", path) != "keep_last")
        throw ConfigError(path + ".history: only keep_last is supported");
    if (!policy.is_valid()) throw ConfigError(path + ".depth: must be between 1 and INT32_MAX");
    return policy;
}

inline DdsConfig parse_dds_config(const YAML::Node& root) {
    using namespace advrf::middleware::config;
    const auto node = section(root, "dds");
    check_keys(node, {"domain", "domain_id", "declare_to_ros", "qos", "robot_name", "namespace"}, "dds");
    DdsConfig cfg;
    cfg.domain_id = unsigned_value(node, node["domain_id"] ? "domain_id" : "domain", 0, "dds");
    cfg.declare_to_ros = value<bool>(node, "declare_to_ros", false, "dds");
    const auto qos = section(node, "qos", "dds");
    if (qos["state"] || qos["command"]) {
        check_keys(qos, {"state", "command"}, "dds.qos");
        cfg.state = parse_endpoint_policy(section(qos, "state", "dds.qos"), "dds.qos.state");
        cfg.command = parse_endpoint_policy(section(qos, "command", "dds.qos"), "dds.qos.command");
    } else if (qos.size() != 0) {
        // Legacy flat QoS now applies to both endpoints instead of being ignored.
        cfg.state = cfg.command = parse_endpoint_policy(qos, "dds.qos");
    }
    return cfg;
}
inline void validate(const DdsConfig& cfg) {
    using advrf::middleware::config::ConfigError;
    if (cfg.domain_id > 232) throw ConfigError("dds.domain_id: must be between 0 and 232");
    if (!cfg.state.is_valid()) throw ConfigError("dds.qos.state.depth: invalid history depth");
    if (!cfg.command.is_valid()) throw ConfigError("dds.qos.command.depth: invalid history depth");
}

struct DdsPluginConfig {
    advrf::middleware::config::CommonConfig common;
    DdsConfig dds;
};
inline DdsPluginConfig parse_dds_plugin_config(const advrf::middleware::config::ConfigFile& file) {
    return {advrf::middleware::config::parse_common_config(file), parse_dds_config(file.root)};
}
inline void validate(const DdsPluginConfig& cfg) {
    advrf::middleware::config::validate(cfg.common);
    validate(cfg.dds);
}

} // namespace advrf::dds_common
