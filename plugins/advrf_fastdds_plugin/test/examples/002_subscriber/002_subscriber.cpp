#include <chrono>
#include <thread>
#include <csignal>
#include <filesystem>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>

#include "advrf_fastdds_plugin/publisher/dds_publisher.hpp"
#include "advrf_middleware_core/config/config_topics.hpp"
#include <advrf_middleware_core/config/robot_config.hpp>
#include <advrf_interfaces/msg/MotorTxPdo.hpp>
#include <advrf_interfaces/msg/MotorTxPdoPubSubTypes.hpp>

using MotorVectorXtTxMsg = advrf_interfaces::msg::dds_::MotorTxPdoVector_;
using MotorXtTxMsg = advrf_interfaces::msg::dds_::MotorTxPdo_;
using MotorXtTxPubSubType = advrf_interfaces::msg::dds_::MotorTxPdoVector_PubSubType;

static volatile std::sig_atomic_t running = 1;

void signal_handler(int)
{
    running = 0;
}

int main(int argc, char** argv)
{
    advrf::log::Log::init();

    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    auto cfg = load_robot_config(ADVRF_CONFIG_SHARE / "middleware" / "config.yaml");
    if (!cfg) return 1;

    config::ConfigTopics topics({cfg->ns, cfg->robot_name});
    auto* participant = eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->create_participant(
        cfg->domain_id,
        eprosima::fastdds::dds::PARTICIPANT_QOS_DEFAULT
    );
    
    if (participant == nullptr) {
        LOG_ERROR("Failed to create DDS participant");
        return 1;
    }

    
    DDSPublisher<MotorVectorXtTxMsg, MotorXtTxPubSubType> publisher;
    if (!publisher.init_dds(topics.command.motorXtCmd(), participant)) {
        LOG_ERROR("Failed to initialize DDS publisher");
        eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->delete_participant(participant);
        return 1;
    }

    LOG_INFO("Publishing MotorXtTxPdo on topic '{}'", topics.command.motorXtCmd());

    while (running)
    {
        MotorVectorXtTxMsg msg;
        MotorXtTxMsg motor_1;
        motor_1.pos_ref() = 1.0f;
        motor_1.vel_ref() = 2.0f;
        motor_1.tor_ref() = 3.0f;
        motor_1.gain_0() = 0.1f;
        motor_1.gain_1() = 0.2f;
        motor_1.gain_2() = 0.3f;
        motor_1.gain_3() = 0.4f;
        motor_1.gain_4() = 0.5f;
        motor_1.fault_ack() = 0;
        motor_1.ts() = 1;
        motor_1.op_idx_aux() = 0;
        motor_1.aux() = 0.0f;
        msg.data().push_back(motor_1);

        MotorXtTxMsg motor_2;
        motor_2.pos_ref() = 10.0f;
        motor_2.vel_ref() = 20.0f;
        motor_2.tor_ref() = 30.0f;
        motor_2.gain_0() = 1.1f;
        motor_2.gain_1() = 1.2f;
        motor_2.gain_2() = 1.3f;
        motor_2.gain_3() = 1.4f;
        motor_2.gain_4() = 1.5f;
        motor_2.fault_ack() = 0;
        motor_2.ts() = 2;
        motor_2.op_idx_aux() = 0;
        motor_2.aux() = 0.0f;
        msg.data().push_back(motor_2);

        publisher.publish(msg);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    LOG_INFO("Stopping publisher");
    eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->delete_participant(participant);

    return 0;
}
