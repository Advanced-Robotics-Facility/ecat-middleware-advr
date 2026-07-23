#include <chrono>
#include <thread>
#include <csignal>

#include <ecat_master_future/shm_utils.hpp> 
#include <ecat_master_future/shm_shared_types.hpp>
#include <advrf_interfaces_protobuf/ecat_pdo.pb.h>
#include <advrf_middleware_core/pdo_utils.hpp>

#include "advrf_cyclonedds_plugin/adapters/dds_adapter_publishers.hpp"

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

    clock_utils::init();
    LOG_INFO("Connecting to shared memory: {}", SHM_NAME);

    DDSAdapterPublishers dds_adapter;
    dds_adapter.shm().connect(SHM_NAME);
    if (!dds_adapter.init(*cfg)) {
        LOG_ERROR("Failed to bind to target DDS channels.");
        return 1;
    }

    while (keep_running && dds_adapter.shm().is_ok()) {
        dds_adapter.spin_once();
        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }

    LOG_INFO("Disconnected from shared memory pipeline. Shutting down.");
    return 0;
}