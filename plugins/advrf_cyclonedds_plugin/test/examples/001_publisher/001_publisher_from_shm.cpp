#include <chrono>
#include <csignal>
#include <filesystem>
#include <thread>
#include <dds/dds.hpp>

#include <advrf_middleware_core/config/config_topics.hpp>
#include <advrf_middleware_core/utils/log.hpp>
#include <advrf_middleware_core/utils/pdo_utils.hpp>

#include "advrf_cyclonedds_plugin/adapters/dds_adapter_publishers.hpp"
#include "advrf_cyclonedds_plugin/config/plugin_config.hpp"

using advrf::cyclonedds_plugin::DDSAdapterPublishers;

namespace
{
    volatile std::sig_atomic_t keep_running = 1;
    void on_signal(int)
    {
        keep_running = 0;
    }
}


int main(int argc, char** argv)
{
    advrf::log::Log::init();

    std::signal(SIGINT, on_signal);
    std::signal(SIGTERM, on_signal);

    auto plugin_config = advrf::cyclonedds_plugin::parse_plugin_config(advrf::middleware::config::ConfigFile::load(advrf::middleware::config::middleware_config_path()));
    advrf::cyclonedds_plugin::validate(plugin_config);
    const auto* config_robot = &plugin_config.common.robot;

    advrf::middleware::clock::init();
    auto config_topics = advrf::middleware::config::ConfigTopics{
            {config_robot->ns, config_robot->robot_name}
    };
    auto domain_participant = dds::domain::DomainParticipant{plugin_config.dds.domain_id};

    advrf::middleware::ecat::EcatDiscover ecat_discover;
    ecat_discover.start(SHM_NRT_RX_PDO);
    auto ecat_map = ecat_discover.discover(advrf::middleware::config::extract_pdo_ids(*config_robot));

    DDSAdapterPublishers dds_adapter;
    if (!dds_adapter.init(
            config_topics,
            *config_robot,
            ecat_map,
            domain_participant, plugin_config.dds))
    {
        LOG_ERROR("Failed to bind to target DDS channels.");
        return 1;
    }

    if (!dds_adapter.start())
    {
        LOG_ERROR("Failed to start DDS publishers adapter.");
        return 1;
    }

    LOG_INFO("DDS publishers adapter started");
    while (keep_running && dds_adapter.is_ok())
    {
        dds_adapter.spin_once();
        std::this_thread::sleep_for(std::chrono::microseconds{100});
    }
    LOG_INFO("Disconnected from shared memory pipeline. Shutting down.");
    return 0;
}
