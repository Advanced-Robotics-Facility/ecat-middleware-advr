#include <iostream>
#include <chrono>
#include <thread>
#include <csignal>

#include <ecat_master_future/shm_utils.hpp> 
#include <ecat_master_future/shm_shared_types.hpp>
#include <advrf_interfaces_protobuf/ecat_pdo.pb.h>
#include <advrf_middleware_core/pdo_utils.hpp>
#include <advrf_middleware_core/utils/log.hpp>

#include "advrf_cyclonedds_plugin/service/dds_adapter_service.hpp"
#include "advrf_cyclonedds_plugin/config/config_topics.hpp"
#include "advrf_middleware_core/robot_config.hpp"

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

    std::unique_ptr<SharedMemoryClient> repl_shm_;
    SharedReplBridge* repl_bridge_;
    while (true) {
        repl_shm_ = std::make_unique<SharedMemoryClient>(SHM_REPL_NAME, sizeof(SharedReplBridge));
        if (repl_shm_->is_valid())
            break;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    repl_bridge_ = repl_shm_->get<SharedReplBridge>();
    while (!repl_bridge_->rt_ready.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    repl_bridge_->mw_ready.store(true);
    
    auto dds_participant = dds::domain::DomainParticipant(cfg->domain_id);

    DDSAdapterService service_server_cmd(
            config::ConfigTopics({"advrf", "robot"}), 
            dds_participant
    );
        
    config::ConfigTopics repl_topics({"advrf", "robot"});
    service_server_cmd.shm().connect();
   
    std::cerr << "[repl_thread] polling..." << std::endl;
    while (keep_running) {
        service_server_cmd.spin_once();
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    return 0;
}