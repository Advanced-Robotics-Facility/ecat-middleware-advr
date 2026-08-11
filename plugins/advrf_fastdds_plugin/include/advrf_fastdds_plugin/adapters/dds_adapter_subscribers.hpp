#pragma once

#include <fastdds/dds/domain/DomainParticipant.hpp>

#include <advrf_interfaces/msg/ReplCmdContent.hpp>
#include <advrf_interfaces/msg/ReplCmdContentPubSubTypes.hpp>

#include "advrf_middleware_core/config/config_topics.hpp"
#include <advrf_middleware_core/adapters/adapter_subscribers.hpp>
#include "advrf_fastdds_plugin/subscriber/dds_subscriber.hpp"
#include <advrf_dds_common/converter/converter.hpp>

using Msg = advrf_interfaces::msg::dds_::ReplCmd_Content_Vector_;
using MsgPubSubType = advrf_interfaces::msg::dds_::ReplCmd_Content_Vector_PubSubType;

class DDSAdapterSubscribers: public middleware_adapter::message::AdapterSubscribers
{
    
public:
    DDSAdapterSubscribers(const config::ConfigTopics& config_topics, 
                          eprosima::fastdds::dds::DomainParticipant* participant);
    void spin_once() override;

protected:
    template <typename Msg, typename MsgPubSubType>
    void register_subscriber(
        const std::string& topic_name,
        eprosima::fastdds::dds::DomainParticipant* participant,
        ChannelTx channel,
        std::function<std::vector<iit::advrf::Ec_slave_pdo>(const Msg&)> converter) 
    {
        auto subscriber = std::make_shared<DDSSubscriber<Msg ,MsgPubSubType>>();
        if (!subscriber->init_dds(topic_name, participant)) {
            LOG_ERROR("Failed to initialize DDS subscriber for topic: {}", topic_name);
            return;
        }

        subscriber->set_callback(
            [this, channel, converter](const Msg& msg) {
                this->forward(std::move(converter(msg)), channel);
            }
        );
        subscribers_.push_back(subscriber);
    }

    template <typename Msg, typename MsgPubSubType>
    void register_subscriber(
        const std::string& topic_name,
        eprosima::fastdds::dds::DomainParticipant* participant,
        ChannelTx channel,
        std::function<iit::advrf::Ec_slave_pdo(const Msg&)> converter) 
    {
        auto subscriber = std::make_shared<DDSSubscriber<Msg ,MsgPubSubType>>();
        if (!subscriber->init_dds(topic_name, participant)) {
            LOG_ERROR("Failed to initialize DDS subscriber for topic: {}", topic_name);
            return;
        }

        subscriber->set_callback(
            [this, channel, converter](const Msg& msg) {
                this->forward(std::move(converter(msg)), channel);
            }
        );
        subscribers_.push_back(subscriber);
    }

private:
    std::vector<std::shared_ptr<DDSSubscriberBase>> subscribers_;
};