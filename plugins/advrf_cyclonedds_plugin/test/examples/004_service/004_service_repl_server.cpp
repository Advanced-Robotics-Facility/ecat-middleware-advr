#include <iostream>
#include <chrono>
#include <thread>
#include <csignal>

#include <ecat_master_future/shm_utils.hpp> 
#include <ecat_master_future/shm_shared_types.hpp>
#include <advrf_interfaces_protobuf/ecat_pdo.pb.h>
#include <advrf_middleware_core/pdo_utils.hpp>
#include <advrf_middleware_core/utils/log.hpp>

#include "advrf_cyclonedds_plugin/publisher.hpp"
#include "advrf_cyclonedds_plugin/service/service_server_cmd.hpp"
#include "advrf_cyclonedds_plugin/config/config_topics.hpp"



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

    DdsAdapter dds_adapter;
    if (!dds_adapter.init(*cfg)) {
        std::cerr << "[SHM-DDS Bridge] Failed to bind to target DDS channels." << std::endl;
        return 1;
    }

    config::ConfigTopics repl_topics({"advrf", "robot"});
    ServiceServerCmd service_server_cmd(repl_topics, 
        dds_adapter.participant());
    service_server_cmd.connect_dds();
    service_server_cmd.connect_shm();

   
    std::cerr << "[repl_thread] polling..." << std::endl;
    while (keep_running) {
        service_server_cmd.spin_once();
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
   

    return 0;
}