#include <chrono>
#include <csignal>
#include <filesystem>
#include <thread>

#include <advrf_middleware_core/config/config_topics.hpp>
#include <advrf_middleware_core/utils/log.hpp>

#include "advrf_cyclonedds_plugin/adapters/dds_adapter_service.hpp"
#include "advrf_cyclonedds_plugin/config/plugin_config.hpp"

using advrf::cyclonedds_plugin::DDSAdapterService;

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

    auto plugin_config = advrf::cyclonedds_plugin::parse_plugin_config(
        advrf::middleware::config::ConfigFile::load(ADVRF_CONFIG_SHARE / "middleware" / "config.yaml"));
    advrf::cyclonedds_plugin::validate(plugin_config);
    const auto* config_robot = &plugin_config.common.robot;
    auto dds_participant = dds::domain::DomainParticipant{plugin_config.dds.domain_id};
    auto config = advrf::middleware::config::ConfigTopics{{config_robot->ns, config_robot->robot_name}};

    DDSAdapterService dds_adapter_service(
        config,
        *config_robot,
        dds_participant, plugin_config.dds);

    if (!dds_adapter_service.start())
    {
        LOG_ERROR("Failed to start DDS service adapter");
        return 1;
    }

    LOG_INFO("DDS service adapter started");
    while (keep_running)
    {
        if (!dds_adapter_service.is_ok())
        {
            LOG_ERROR("DDS service adapter connection lost");
            break;
        }
        dds_adapter_service.spin_once();
        std::this_thread::sleep_for(
            std::chrono::milliseconds{100});
    }

    dds_adapter_service.close();

    return 0;
}
