#include <chrono>
#include <thread>
#include <csignal>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>

#include "advrf_fastdds_plugin/publisher/dds_publisher.hpp"
#include "advrf_middleware_core/config/config_topics.hpp"

#include <advrf_interfaces/msg/CtrlCmd.hpp>
#include <advrf_interfaces/msg/CtrlCmdPubSubTypes.hpp>

using Msg = advrf_interfaces::msg::dds_::CtrlCmd_;
using MsgPubSubType = advrf_interfaces::msg::dds_::CtrlCmd_PubSubType;

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

    auto* participant = eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->create_participant(
        42,
        eprosima::fastdds::dds::PARTICIPANT_QOS_DEFAULT
    );
    if (participant == nullptr) {
        LOG_ERROR("Failed to create DDS participant");
        return 1;
    }

    config::ConfigTopics topics({"advrf", "kyon"});

    DDSPublisher<Msg, MsgPubSubType> publisher;

    if (!publisher.init_dds(topics.command.jointCmd(), participant)) {
        LOG_ERROR("Failed to initialize DDS publisher");
        eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->delete_participant(participant);
        return 1;
    }

    LOG_INFO("Publishing CtrlCmd on topic '{}'", topics.command.jointCmd());

    while (running)
    {
        Msg msg;

        msg.type() = 18; // CTRL_SET_POSITION
        msg.board_id() = 51;
        msg.value() = 0.0;

        publisher.publish(msg);

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    LOG_INFO("Stopping publisher");
    eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->delete_participant(participant);

    return 0;
}