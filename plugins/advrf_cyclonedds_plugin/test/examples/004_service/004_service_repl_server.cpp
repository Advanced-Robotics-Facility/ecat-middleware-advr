#include <iostream>
#include <chrono>
#include <thread>
#include <csignal>

#include <ecat_master_future/shm_utils.hpp> 
#include <ecat_master_future/shm_shared_types.hpp>
#include <advrf_interfaces_protobuf/ecat_pdo.pb.h>
#include <advrf_middleware_core/utils/pdo_utils.hpp>
#include <advrf_middleware_core/utils/log.hpp>
#include <advrf_middleware_core/config/robot_config.hpp>

#include "advrf_cyclonedds_plugin/adapters/dds_adapter_service.hpp"
#include "advrf_cyclonedds_plugin/config/config_topics.hpp"

#include <advrf_middleware_core/shared_memory/shm_connection_repl.hpp>

namespace {
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

    auto cfg = load_robot_config(ROBOT_CONFIG_DIR);
    if (!cfg) return 1;
    
    auto dds_participant = dds::domain::DomainParticipant(cfg->domain_id);
    auto config = config::ConfigTopics({"advrf", "robot"});
    DDSAdapterService dds_adapter_service(config, dds_participant);
        
    dds_adapter_service.shm().connect(SHM_REPL_NAME);

    while (keep_running) {
        dds_adapter_service.spin_once();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    dds_adapter_service.shm().close();

    return 0;
}