#include <filesystem>
#include <dds/dds.hpp>

#include <advrf_middleware_core/config/robot_config.hpp>
#include <advrf_middleware_core/utils/ecat_discover.hpp>
#include <advrf_middleware_core/utils/log.hpp>

#include "advrf_cyclonedds_plugin/config/plugin_config.hpp"

int main(int argc, char** argv)
{
    advrf::log::Log::init();
    auto plugin_config = advrf::cyclonedds_plugin::parse_plugin_config(
        advrf::middleware::config::ConfigFile::load(ADVRF_CONFIG_SHARE / "middleware" / "config.yaml"));
    advrf::cyclonedds_plugin::validate(plugin_config);
    const auto* config_robot = &plugin_config.common.robot;

    advrf::middleware::ecat::EcatDiscover ecat_discover;
    if (!ecat_discover.start(SHM_NRT_RX_PDO)) {
        LOG_ERROR("Failed to connect to shared memory: {}", SHM_NRT_RX_PDO);
        return 1;
    }

    auto ecat_map = ecat_discover.discover(advrf::middleware::config::extract_pdo_ids(*config_robot));

    LOG_INFO("Ecat discovery finished");
    for(auto [id, metadata] : ecat_map) {
        LOG_INFO("Discovered PDO ID: {} Name: {} Type: {} Channel: {} Device: {}", 
            id, metadata.name, static_cast<int>(metadata.type), 
            static_cast<int>(metadata.channel), static_cast<int>(metadata.device));
    }

    return 0;
}