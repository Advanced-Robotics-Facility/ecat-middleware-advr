#pragma once

#include <advrf_interfaces/msg/ReplCmdContent.hpp>
#include <advrf_middleware_core/adapters/adapter_subscribers.hpp>
#include <advrf_dds_common/converter/converter.hpp>
#include <advrf_dds_common/qos/reader_policy.hpp>

#include "advrf_middleware_core/config/config_topics.hpp"
#include "advrf_cyclonedds_plugin/subscriber/dds_subscriber.hpp"

class DDSAdapterSubscribers: public middleware_adapter::message::AdapterSubscribers
{
public:
    DDSAdapterSubscribers(
        const config::ConfigTopics& config_topics,
        dds::domain::DomainParticipant& participant,
        advrf::dds_common::ReaderPolicy reader_policy = {});

    void spin_once() override;

    bool is_initialized() const noexcept { return initialized_; }

protected:

    template <typename Msg>
    void register_subscriber(
        const std::string& topic_name,
        dds::domain::DomainParticipant& participant,
        ChannelTx channel,
        std::function<std::vector<iit::advrf::Ec_slave_pdo>(const Msg&)> converter)
    {
        auto subscriber = std::make_shared<DDSSubscriber<Msg>>();
        if (!subscriber->init_dds(topic_name, participant, reader_policy_)) {
            LOG_ERROR(
                "Failed to initialize DDS subscriber for topic: {}",
                topic_name);
            initialized_ = false;
            return;
        }

        subscriber->set_callback(
            [this, channel, converter](const Msg& msg)
            {
                for(const auto& pdo : converter(msg)){
                    this->push(channel, pdo);
                }
            });
        subscribers_.push_back(subscriber);
    }


    template <typename Msg>
    void register_subscriber(
        const std::string& topic_name,
        dds::domain::DomainParticipant& participant,
        ChannelTx channel,
        std::function<iit::advrf::Ec_slave_pdo(const Msg&)> converter){
        auto subscriber = std::make_shared<DDSSubscriber<Msg>>();
        if (!subscriber->init_dds(topic_name, participant, reader_policy_)) {
            LOG_ERROR(
                "Failed to initialize DDS subscriber for topic: {}",
                topic_name);
            initialized_ = false;
            return;
        }

        subscriber->set_callback(
            [this, channel, converter](const Msg& msg)
            {
                this->push(channel, std::move(converter(msg)));
            });
        subscribers_.push_back(subscriber);
    }
    

private:
    bool initialized_{true};
    advrf::dds_common::ReaderPolicy reader_policy_;
    std::vector<std::shared_ptr<DDSSubscriberBase>> subscribers_;
};