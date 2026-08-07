#pragma once

#include <advrf_interfaces/msg/ReplCmdContent.hpp>
#include <advrf_middleware_core/adapters/adapter_subscribers.hpp>

#include "advrf_middleware_core/config/config_topics.hpp"
#include "advrf_cyclonedds_plugin/subscriber/dds_subscriber.hpp"

using MessageDDS = advrf_interfaces::msg::dds_::ReplCmd_Content_Vector_;
using MessageProtobuf = iit::advrf::Repl_cmd_vector;

class DDSAdapterSubscribers: public middleware_adapter::message::AdapterSubscribers
{
    
public:
    DDSAdapterSubscribers(const config::ConfigTopics& config_topics, dds::domain::DomainParticipant& participant);
    void spin_once() override;

private:
    DDSSubscriber<MessageDDS> subscriber_;
};