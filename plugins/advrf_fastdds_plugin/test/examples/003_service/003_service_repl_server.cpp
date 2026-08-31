#include <chrono>
#include <thread>
#include <csignal>
#include <filesystem>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>

#include <shm_utils.hpp>

#include <advrf_middleware_core/utils/pdo_utils.hpp>
#include <advrf_middleware_core/utils/log.hpp>
#include <advrf_middleware_core/config/robot_config.hpp>

#include "advrf_fastdds_plugin/adapters/dds_adapter_service.hpp"
#include "advrf_middleware_core/config/config_topics.hpp"


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

    auto config_robot = config::load_robot_config(
      ADVRF_CONFIG_SHARE / "robot_id_map" / "robot_id_map.yaml",
      ADVRF_CONFIG_SHARE / "robot_ecat" / "ecat_config.yaml");
    if (!config_robot) return 1;

    auto config = config::ConfigTopics({config_robot->ns, config_robot->robot_name});
    auto* participant = eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->create_participant(
        config_robot->domain_id,
        eprosima::fastdds::dds::PARTICIPANT_QOS_DEFAULT
    );
    if (participant == nullptr) {
        LOG_ERROR("Failed to create DDS participant");
        return 1;
    }

    advrf::fastdds_plugin::DDSAdapterService dds_adapter_service(config, *config_robot, participant);
    if(!dds_adapter_service.shm().connect(SHM_SERVICE, ShmAttachMode::Open))
    {
        LOG_ERROR("Failed to connect to shared memory");
        return 1;
    }

    while (keep_running) {
        dds_adapter_service.spin_once();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    dds_adapter_service.shm().close();
    eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->delete_participant(participant);

    return 0;
}