#pragma once

#include <advrf_interfaces/msg/MotorsPdoCmd.hpp>
#include <advrf_interfaces_protobuf/repl_cmd.pb.h>
#include <advrf_middleware_core/adapters/adapter_subscribers.hpp>

#include "advrf_cyclonedds_plugin/config/config_topics.hpp"
#include "advrf_cyclonedds_plugin/subscriber/dds_subscriber.hpp"

using MessageDDS = advrf_interfaces::msg::dds_::MotorsPdoCmd_;
using MessageProtobuf = iit::advrf::Motors_PDO_cmd;

class DDSAdapterSubscribers: public middleware_adapter::message::AdapterSubscribers
{
    
public:
    DDSAdapterSubscribers(const config::ConfigTopics& config_topics, dds::domain::DomainParticipant& participant);
    void spin_once() override;

private:
    DDSSubscriber<MessageDDS> subscriber_;
};