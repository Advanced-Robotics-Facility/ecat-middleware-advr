#include <chrono>
#include <csignal>
#include <filesystem>
#include <memory>
#include <thread>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>

#include <advrf_middleware_core/utils/pdo_utils.hpp>
#include <shm_utils.hpp>

#include "advrf_fastdds_plugin/adapters/dds_adapter_publishers.hpp"
#include "advrf_fastdds_plugin/config/plugin_config.hpp"

namespace {
volatile std::sig_atomic_t keep_running = 1;
void on_signal(int) { keep_running = 0; }
}

int main(int argc, char** argv)
{
    advrf::log::Log::init();
    std::signal(SIGINT, on_signal);
    std::signal(SIGTERM, on_signal);

    auto plugin_config = advrf::fastdds_plugin::parse_plugin_config(
        advrf::middleware::config::ConfigFile::load(ADVRF_CONFIG_SHARE / "middleware" / "config.yaml"));
    advrf::fastdds_plugin::validate(plugin_config);
    const auto* config_robot = &plugin_config.common.robot;
    advrf::middleware::ecat::EcatDiscover ecat_discover;
    ecat_discover.start(SHM_NRT_RX_PDO);
    auto ecat_map = ecat_discover.discover(advrf::middleware::config::extract_pdo_ids(*config_robot));

    advrf::middleware::clock::init();
    auto config = advrf::middleware::config::ConfigTopics{{config_robot->ns, config_robot->robot_name}};

    auto* domain_participant = eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->create_participant(
        plugin_config.dds.domain_id,
        eprosima::fastdds::dds::PARTICIPANT_QOS_DEFAULT
    );
    if (domain_participant == nullptr) {
        LOG_ERROR("Failed to create DDS DomainParticipant.");
        return 1;
    }
    
    auto delete_participant = [](eprosima::fastdds::dds::DomainParticipant* participant) {
        participant->delete_contained_entities();
        eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->delete_participant(participant);
    };
    std::unique_ptr<eprosima::fastdds::dds::DomainParticipant, decltype(delete_participant)>
        participant_owner(domain_participant, delete_participant);

    // DDS adapter resources must be released before the participant.
    advrf::fastdds_plugin::DDSAdapterPublishers dds_adapter;
    if (!dds_adapter.init(config, *config_robot, ecat_map, domain_participant, plugin_config.dds)) {
        LOG_ERROR("Failed to bind to target DDS channels.");
        return 1;
    }

    if (!dds_adapter.start()) {
        LOG_ERROR("Failed to start DDS publishers adapter.");
        return 1;
    }

    while (keep_running && dds_adapter.is_ok()) {
        dds_adapter.spin_once();
        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }

    LOG_INFO("Disconnected from shared memory pipeline. Shutting down.");
    dds_adapter.close();

    return 0;
}