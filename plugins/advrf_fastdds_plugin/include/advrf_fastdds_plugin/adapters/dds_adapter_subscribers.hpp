#pragma once

#include <fastdds/dds/domain/DomainParticipant.hpp>

#include <advrf_interfaces/msg/ReplCmdContent.hpp>
#include <advrf_interfaces/msg/ReplCmdContentPubSubTypes.hpp>
#include <advrf_interfaces_protobuf/repl_cmd.pb.h>
#include <advrf_middleware_core/adapters/adapter_subscribers.hpp>

#include "advrf_dds_common/config/config_topics.hpp"
#include "advrf_fastdds_plugin/subscriber/dds_subscriber.hpp"

using MessageDDS = advrf_interfaces::msg::dds_::ReplCmd_Content_Vector_;
using MessageDDSPubSubType = advrf_interfaces::msg::dds_::ReplCmd_Content_Vector_PubSubType;
using MessageProtobuf = iit::advrf::Repl_cmd_vector;

class DDSAdapterSubscribers: public middleware_adapter::message::AdapterSubscribers
{
    
public:
    DDSAdapterSubscribers(const config::ConfigTopics& config_topics, 
                          eprosima::fastdds::dds::DomainParticipant* participant);
    void spin_once() override;

private:
    DDSSubscriber<MessageDDS, MessageDDSPubSubType> subscriber_;
};