#include <chrono>
#include <thread>
#include <csignal>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>

#include <shm_utils.hpp>
#include <advrf_interfaces_protobuf/ecat_pdo.pb.h>
#include <advrf_middleware_core/utils/pdo_utils.hpp>
#include <advrf_middleware_core/config/robot_config.hpp>

#include "advrf_fastdds_plugin/adapters/dds_adapter_publishers.hpp"

namespace {
volatile std::sig_atomic_t keep_running = 1;
void on_signal(int) { keep_running = 0; }
}

int main(int argc, char** argv)
{
    advrf::log::Log::init();
    std::signal(SIGINT, on_signal);
    std::signal(SIGTERM, on_signal);

    auto cfg = load_robot_config(ROBOT_CONFIG_DIR);
    if (!cfg) return 1;

    clock_utils::init();
    DDSAdapterPublishers dds_adapter;
    dds_adapter.shm().connect(SHM_NRT_RX_PDO);

    auto config = config::ConfigTopics({"advrf", cfg->robot_name});

    auto* domain_participant = eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->create_participant(
        cfg->domain_id,
        eprosima::fastdds::dds::PARTICIPANT_QOS_DEFAULT
    );
    if (domain_participant == nullptr) {
        LOG_ERROR("Failed to create DDS DomainParticipant.");
        return 1;
    }
    
    if (!dds_adapter.init(config, *cfg, domain_participant)) {
        LOG_ERROR("Failed to bind to target DDS channels.");
        return 1;
    }

    while (keep_running && dds_adapter.shm().is_ok()) {
        dds_adapter.spin_once();
        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }

    LOG_INFO("Disconnected from shared memory pipeline. Shutting down.");
    eprosima::fastdds::dds::DomainParticipantFactory::get_instance()
    ->delete_participant(domain_participant);

    return 0;
}