#include <chrono>
#include <thread>
#include <csignal>
#include <filesystem>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>

#include <shm_utils.hpp>
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

    auto config_robot = config::load_robot_config(
      ADVRF_CONFIG_SHARE / "robot_id_map" / "robot_id_map.yaml",
      ADVRF_CONFIG_SHARE / "robot_ecat" / "ecat_config.yaml");
    if (!config_robot) return 1;

    EcatDiscover ecat_discover;
    ecat_discover.start(SHM_NRT_RX_PDO);
    auto ecat_map = ecat_discover.discover(config::extract_pdo_ids(*config_robot));

    clock_utils::init();
    advrf::fastdds_plugin::DDSAdapterPublishers dds_adapter;
    if(!dds_adapter.shm().connect(SHM_NRT_RX_PDO, ShmAttachMode::Open)) {
        LOG_ERROR("Failed to connect to shared memory");
        return 1;
    }

    auto config = config::ConfigTopics{{config_robot->ns, config_robot->robot_name}};

    auto* domain_participant = eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->create_participant(
        config_robot->domain_id,
        eprosima::fastdds::dds::PARTICIPANT_QOS_DEFAULT
    );
    if (domain_participant == nullptr) {
        LOG_ERROR("Failed to create DDS DomainParticipant.");
        return 1;
    }
    
    if (!dds_adapter.init(config, *config_robot, ecat_map,domain_participant)) {
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