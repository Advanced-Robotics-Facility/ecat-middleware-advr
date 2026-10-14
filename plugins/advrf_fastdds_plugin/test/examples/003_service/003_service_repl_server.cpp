#include <chrono>
#include <csignal>
#include <filesystem>
#include <memory>
#include <thread>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>

#include <advrf_middleware_core/config/config_topics.hpp>
#include <advrf_middleware_core/utils/log.hpp>
#include <advrf_middleware_core/utils/pdo_utils.hpp>
#include <shm_utils.hpp>

#include "advrf_fastdds_plugin/adapters/dds_adapter_service.hpp"
#include "advrf_fastdds_plugin/config/plugin_config.hpp"

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

    auto plugin_config = advrf::fastdds_plugin::parse_plugin_config(
        advrf::middleware::config::ConfigFile::load(ADVRF_CONFIG_SHARE / "middleware" / "config.yaml"));
    advrf::fastdds_plugin::validate(plugin_config);
    const auto* config_robot = &plugin_config.common.robot;
    auto config = advrf::middleware::config::ConfigTopics({config_robot->ns, config_robot->robot_name});
    auto* participant = eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->create_participant(
        plugin_config.dds.domain_id,
        eprosima::fastdds::dds::PARTICIPANT_QOS_DEFAULT
    );
    if (participant == nullptr) {
        LOG_ERROR("Failed to create DDS participant");
        return 1;
    }

    auto delete_participant = [](eprosima::fastdds::dds::DomainParticipant* participant) {
        participant->delete_contained_entities();
        eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->delete_participant(participant);
    };
    std::unique_ptr<eprosima::fastdds::dds::DomainParticipant, decltype(delete_participant)>
        participant_owner(participant, delete_participant);

    advrf::fastdds_plugin::DDSAdapterService dds_adapter_service(config, *config_robot, participant, plugin_config.dds);
    if(!dds_adapter_service.start())
    {
        LOG_ERROR("Failed to start DDS service adapter.");
        return 1;
    }

    while (keep_running && dds_adapter_service.is_ok()) {
        dds_adapter_service.spin_once();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    dds_adapter_service.close();

    return 0;
}