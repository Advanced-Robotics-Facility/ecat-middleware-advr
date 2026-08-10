#pragma once

#include <advrf_interfaces/msg/ReplCmdContent.hpp>
#include <advrf_middleware_core/adapters/adapter_subscribers.hpp>
#include <advrf_dds_common/converter/converter.hpp>

#include "advrf_middleware_core/config/config_topics.hpp"
#include "advrf_cyclonedds_plugin/subscriber/dds_subscriber.hpp"

using MessageProtobuf = iit::advrf::Repl_cmd_vector;

class DDSAdapterSubscribers: public middleware_adapter::message::AdapterSubscribers
{


public:
    DDSAdapterSubscribers(const config::ConfigTopics& config_topics, dds::domain::DomainParticipant& participant);
    void spin_once() override;

protected:

    template <typename Msg>
    void register_subscriber(
        const std::string& topic_name,
        dds::domain::DomainParticipant& participant,
        ChannelTx channel)
    {
        register_subscriber<Msg>(
            topic_name,
            participant,
            channel,
            [](const Msg& msg) {
                return std::array<const Msg*, 1>{&msg};
            });
    }

    template <typename Msg, typename Extractor>
    void register_subscriber(
        const std::string& topic_name,
        dds::domain::DomainParticipant& participant,
        ChannelTx channel,
        Extractor extractor){
        auto subscriber = std::make_shared<DDSSubscriber<Msg>>();
        if (!subscriber->init_dds(topic_name, participant)) {
            LOG_ERROR(
                "Failed to initialize DDS subscriber for topic: {}",
                topic_name);
            return;
        }

        subscriber->set_callback(
            [this, channel, extractor](const Msg& msg)
            {
                const auto& messages = extractor(msg);
                for (const auto& data : messages) {
                    iit::advrf::Ec_slave_pdo pdo;
                    convert::protobuf::from_dds(data, pdo);
                    this->forward(pdo, channel);
                }
            });
        subscribers_.push_back(subscriber);
    }

private:
    std::vector<std::shared_ptr<DDSSubscriberBase>> subscribers_;
};