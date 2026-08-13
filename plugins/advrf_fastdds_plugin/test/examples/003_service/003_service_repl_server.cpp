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

    auto cfg = load_robot_config(ADVRF_CONFIG_SHARE / "middleware" / "config.yaml");
    if (!cfg) return 1;
    
    auto config = config::ConfigTopics({cfg->ns, cfg->robot_name});
    auto* participant = eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->create_participant(
        cfg->domain_id,
        eprosima::fastdds::dds::PARTICIPANT_QOS_DEFAULT
    );
    if (participant == nullptr) {
        LOG_ERROR("Failed to create DDS participant");
        return 1;
    }

    DDSAdapterService dds_adapter_service(config, participant);
        
    if(!dds_adapter_service.shm().connect(SHM_REPL_NAME, ShmAttachMode::Open))
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