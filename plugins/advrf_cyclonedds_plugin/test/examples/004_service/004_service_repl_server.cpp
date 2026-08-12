#include <chrono>
#include <csignal>
#include <thread>
#include <filesystem>

#include <advrf_middleware_core/config/robot_config.hpp>
#include <advrf_middleware_core/config/config_topics.hpp>
#include <advrf_middleware_core/utils/log.hpp>

#include "advrf_cyclonedds_plugin/adapters/dds_adapter_service.hpp"


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


    const auto cfg = load_robot_config( ADVRF_CONFIG_SHARE / "middleware" / "middleware.yaml");
    auto dds_participant = dds::domain::DomainParticipant{cfg->domain_id};
    auto config =config::ConfigTopics{{cfg->ns, cfg->robot_name}};

    if (!cfg)
        return 1;

    DDSAdapterService dds_adapter_service{
        config,
        dds_participant};

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


    dds_adapter_service.shm().close();

    return 0;
}
