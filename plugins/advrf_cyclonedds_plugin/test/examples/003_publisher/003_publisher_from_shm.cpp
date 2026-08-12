#include <chrono>
#include <csignal>
#include <filesystem>
#include <thread>

#include <advrf_middleware_core/config/robot_config.hpp>
#include <advrf_middleware_core/utils/log.hpp>
#include <advrf_middleware_core/utils/pdo_utils.hpp>

#include "advrf_cyclonedds_plugin/adapters/dds_adapter_publishers.hpp"

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

    if (!cfg)
        return 1;


    clock_utils::init();


    auto config =
        config::ConfigTopics{
            {cfg->ns, cfg->robot_name}
        };

    auto domain_participant =
        dds::domain::DomainParticipant{
            cfg->domain_id
        };


    DDSAdapterPublishers dds_adapter;


    if (!dds_adapter.init(
            config,
            *cfg,
            domain_participant))
    {
        LOG_ERROR("Failed to bind to target DDS channels.");
        return 1;
    }


    /*
     * AdapterPublishers::start() owns the SHM connection:
     *
     *   ShmRxReader
     *   -> Open SHM_NRT_RX_PDO
     *   -> wait for ShmRxWriter readiness
     */
    if (!dds_adapter.start())
    {
        LOG_ERROR("Failed to start DDS publishers adapter.");
        return 1;
    }


    LOG_INFO("DDS publishers adapter started");


    while (keep_running && dds_adapter.is_ok())
    {
        dds_adapter.spin_once();

        std::this_thread::sleep_for(
            std::chrono::microseconds{100});
    }


    LOG_INFO(
        "Disconnected from shared memory pipeline. Shutting down.");

    return 0;
}