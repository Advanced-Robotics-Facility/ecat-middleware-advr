#include <dds/dds.hpp>
#include <filesystem>

#include "advrf_middleware_core/config/robot_config.hpp"
#include "advrf_middleware_core/utils/log.hpp"
#include "advrf_middleware_core/utils/pdo_discover.hpp" 

int main(int argc, char** argv)
{
    advrf::log::Log::init();
    const auto cfg = load_robot_config( ADVRF_CONFIG_SHARE / "middleware" / "config.yaml");

    PdoDiscover pdo_discover;
    if (!pdo_discover.start(SHM_NRT_RX_PDO)) {
        LOG_ERROR("Failed to connect to shared memory: {}", SHM_NRT_RX_PDO);
        return 1;
    }

    auto pdo_map = pdo_discover.discover(extract_pdo_ids(*cfg));

    LOG_INFO("Pdo discovery finished");
    for(auto [id, metadata] : pdo_map) {
        LOG_INFO("Discovered PDO ID: {} Name: {} Type: {} Channel: {} Device: {}", 
            id, metadata.name, static_cast<int>(metadata.type), 
            static_cast<int>(metadata.channel), static_cast<int>(metadata.device));
    }

    return 0;
}