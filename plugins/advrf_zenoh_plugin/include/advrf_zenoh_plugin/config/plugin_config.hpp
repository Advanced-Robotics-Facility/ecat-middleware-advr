#pragma once

#include <advrf_middleware_core/config/runtime_config.hpp>

#include "advrf_zenoh_plugin/config/wire_format.hpp"

namespace advrf::zenoh_plugin {

inline WireFormat parse_wire_format(const std::string& text) {
    using advrf::middleware::config::ConfigError;
    if (text == "protobuf") return WireFormat::Protobuf;
    if (text == "ros2cdr") return WireFormat::Ros2Cdr;
    throw ConfigError("zenoh.wire_format: expected protobuf or ros2cdr");
}
struct ZenohConfig {
    std::filesystem::path session_config;
    WireFormat wire_format = WireFormat::Protobuf;
};
struct PluginConfig {
    advrf::middleware::config::CommonConfig common;
    ZenohConfig transport;
};
inline PluginConfig parse_plugin_config(const advrf::middleware::config::ConfigFile& file) {
    using namespace advrf::middleware::config;
    PluginConfig cfg;
    cfg.common = parse_common_config(file);
    const auto node = section(file.root, "zenoh");
    check_keys(node, {"session_config", "wire_format"}, "zenoh");
    const auto path = value<std::string>(node, "session_config", "", "zenoh");
    if (!path.empty()) cfg.transport.session_config = file.resolve(path);
    cfg.transport.wire_format = parse_wire_format(value<std::string>(node, "wire_format", "protobuf", "zenoh"));
    return cfg;
}
inline void validate(const PluginConfig& cfg) {
    advrf::middleware::config::validate(cfg.common);
#ifndef ZENOH_ROS2_SUPPORT
    if (cfg.transport.wire_format == WireFormat::Ros2Cdr)
        throw advrf::middleware::config::ConfigError("zenoh.wire_format: ros2cdr requires ZENOH_ROS2_SUPPORT=ON");
#endif
    if (!cfg.transport.session_config.empty() && !std::filesystem::is_regular_file(cfg.transport.session_config))
        throw advrf::middleware::config::ConfigError("zenoh.session_config: file does not exist: " + cfg.transport.session_config.string());
}

} // namespace advrf::zenoh_plugin
