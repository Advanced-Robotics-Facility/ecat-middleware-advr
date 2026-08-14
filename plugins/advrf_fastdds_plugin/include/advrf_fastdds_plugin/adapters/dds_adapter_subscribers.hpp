#pragma once

#include <fastdds/dds/domain/DomainParticipant.hpp>

#include <advrf_interfaces/msg/ReplCmdContent.hpp>
#include <advrf_interfaces/msg/ReplCmdContentPubSubTypes.hpp>

#include "advrf_fastdds_plugin/subscriber/dds_subscriber.hpp"
#include "advrf_fastdds_plugin/ros_metadata/ros_graph_bridge.hpp"

#include <advrf_middleware_core/config/robot_config.hpp>
#include <advrf_middleware_core/config/config_topics.hpp>
#include <advrf_middleware_core/adapters/adapter_subscribers.hpp>
#include <advrf_dds_common/converter/converter.hpp>
#include <advrf_dds_common/qos/reader_policy.hpp>

using Msg = advrf_interfaces::msg::dds_::ReplCmd_Content_Vector_;
using MsgPubSubType = advrf_interfaces::msg::dds_::ReplCmd_Content_Vector_PubSubType;


template <typename Msg, typename MsgPubSubType>
class DDSAdapterBridgeSubscriber
    : public IConnectRosGraphBridge,
      public DDSSubscriber<Msg,MsgPubSubType>
{
public:
    void connect_ros_graph_bridge(FastRosGraphBridge& bridge) override
    {
        bridge.add_reader(this->dds_reader());
    }
};


class DDSAdapterSubscribers: public middleware_adapter::message::AdapterSubscribers
{
    
public:
    DDSAdapterSubscribers(const config::ConfigTopics& config_topics, 
                            const RobotConfig&,
                          eprosima::fastdds::dds::DomainParticipant* participant,
                          advrf::dds_common::ReaderPolicy reader_policy = {});

    void spin_once() override;

    bool is_initialized() const noexcept { return initialized_; }

protected:
    template <typename Msg, typename MsgPubSubType>
    void register_subscriber(
        const std::string& topic_name,
        eprosima::fastdds::dds::DomainParticipant* participant,
        ChannelTx channel,
        std::function<std::vector<iit::advrf::Ec_slave_pdo>(const Msg&)> converter)
    {
        auto subscriber =
            std::make_shared<DDSAdapterBridgeSubscriber<Msg, MsgPubSubType>>();

        if (!subscriber->init_dds(
                topic_name,
                participant,
                reader_policy_))
        {
            LOG_ERROR(
                "Failed to initialize DDS subscriber for topic: {}",
                topic_name
            );

            initialized_ = false;
            return;
        }

        subscriber->set_callback(
            [this, channel, converter = std::move(converter)](
                const Msg& msg)
            {
                for (const auto& pdo : converter(msg)) {
                    push(channel, pdo);
                }
            }
        );

        ros_connectables_.emplace_back(*subscriber);
        subscribers_.emplace_back(std::move(subscriber));
    }

    template <typename Msg, typename MsgPubSubType>
    void register_subscriber(
        const std::string& topic_name,
        eprosima::fastdds::dds::DomainParticipant* participant,
        ChannelTx channel,
        std::function<iit::advrf::Ec_slave_pdo(const Msg&)> converter) 
    {
        auto subscriber = std::make_shared<DDSSubscriber<Msg ,MsgPubSubType>>();
        if (!subscriber->init_dds(topic_name, participant, reader_policy_)) {
            LOG_ERROR("Failed to initialize DDS subscriber for topic: {}", topic_name);
            initialized_ = false;
            return;
        }

        subscriber->set_callback(
            [this, channel, converter](const Msg& msg) {
                this->push(channel, std::move(converter(msg)));
            }
        );
        subscribers_.push_back(subscriber);
    }

    void init_ros_graph_bridge(
        const RobotConfig& robot_config,
        eprosima::fastdds::dds::DomainParticipant* participant
    );

private:
    advrf::dds_common::ReaderPolicy reader_policy_;
    bool initialized_{true};
    std::vector<std::shared_ptr<DDSSubscriberBase>> subscribers_;

    std::unique_ptr<FastRosGraphBridge> ros_graph_bridge_;
    std::vector<std::reference_wrapper<IConnectRosGraphBridge>> ros_connectables_;
};