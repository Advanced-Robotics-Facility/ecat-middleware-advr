#include <dds/dds.hpp>
#include <filesystem>

#include "advrf_middleware_core/config/robot_config.hpp"
#include "advrf_middleware_core/utils/log.hpp"
#include "advrf_middleware_core/utils/ecat_discover.hpp" 

int main(int argc, char** argv)
{
    advrf::log::Log::init();
    auto config_robot = config::load_robot_config(
        ADVRF_CONFIG_SHARE / "robot_id_map" / "robot_id_map.yaml",
        ADVRF_CONFIG_SHARE / "robot_ecat" / "ecat_config.yaml");

    EcatDiscover ecat_discover;
    if (!ecat_discover.start(SHM_NRT_RX_PDO)) {
        LOG_ERROR("Failed to connect to shared memory: {}", SHM_NRT_RX_PDO);
        return 1;
    }

    auto ecat_map = ecat_discover.discover(config::extract_pdo_ids(*config_robot));

    LOG_INFO("Ecat discovery finished");
    for(auto [id, metadata] : ecat_map) {
        LOG_INFO("Discovered PDO ID: {} Name: {} Type: {} Channel: {} Device: {}", 
            id, metadata.name, static_cast<int>(metadata.type), 
            static_cast<int>(metadata.channel), static_cast<int>(metadata.device));
    }

    return 0;
}