#pragma once
#include <advrf_dds_common/config/dds_config.hpp>

namespace advrf::fastdds_plugin {
struct PluginConfig {
    advrf::middleware::config::CommonConfig common;
    advrf::dds_common::DdsConfig dds;
};
inline PluginConfig parse_plugin_config(const advrf::middleware::config::ConfigFile& file) {
    const auto cfg = advrf::dds_common::parse_dds_plugin_config(file);
    return {cfg.common, cfg.dds};
}
inline void validate(const PluginConfig& cfg) {
    advrf::middleware::config::validate(cfg.common);
    advrf::dds_common::validate(cfg.dds);
}
} // namespace advrf::fastdds_plugin
