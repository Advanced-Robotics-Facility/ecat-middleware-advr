#pragma once

#include <dds/dds.hpp>

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include <advrf_interfaces/msg/ReplCmdContent.hpp>

#include <advrf_dds_common/converter/converter.hpp>
#include <advrf_dds_common/qos/reader_policy.hpp>

#include <advrf_middleware_core/adapters/adapter_subscribers.hpp>
#include <advrf_middleware_core/config/config_topics.hpp>
#include <advrf_middleware_core/config/robot_config.hpp>
#include <advrf_middleware_core/utils/ecat_discover.hpp>

#include "advrf_cyclonedds_plugin/ros_metadata/ros_graph_bridge.hpp"
#include "advrf_cyclonedds_plugin/subscriber/dds_subscriber.hpp"

template <typename Msg>
class DDSPdoSubscriber
    : public IConnectRosGraphBridge,
      public DDSSubscriber<Msg>
{
public:
    void connect_ros_graph_bridge(CycloneDDSRosGraphBridge& bridge) override
    {
        bridge.add_reader(this->dds_reader());
    }
};


class DDSAdapterSubscribers
    : public middleware_adapter::message::AdapterSubscribers
{
public:

    bool init(
        const config::ConfigTopics& config_topics,
        const RobotConfig& robot_config,
        const EcatDiscover::EcatMap &ecat_map,
        dds::domain::DomainParticipant& participant,
        advrf::dds_common::ReaderPolicy reader_policy = {}
    );

    void spin_once() override;

private:
    template <typename Msg>
    bool register_subscriber(
        const std::string& topic_name,
        dds::domain::DomainParticipant& participant,
        ChannelTx channel,
        std::function<std::vector<iit::advrf::Ec_slave_pdo>(const Msg&)> converter)
    {
        auto subscriber =
            std::make_shared<DDSPdoSubscriber<Msg>>();

        if (!subscriber->init_dds(
                topic_name,
                participant,
                reader_policy_))
        {
            LOG_ERROR(
                "Failed to initialize DDS subscriber for topic: {}",
                topic_name
            );

            return false;
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
        return true;
    }

    template <typename Msg>
    bool register_subscriber(
        const std::string& topic_name,
        dds::domain::DomainParticipant& participant,
        ChannelTx channel,
        std::function<iit::advrf::Ec_slave_pdo(const Msg&)> converter)
    {
        auto subscriber =
            std::make_shared<DDSPdoSubscriber<Msg>>();

        if (!subscriber->init_dds(
                topic_name,
                participant,
                reader_policy_))
        {
            LOG_ERROR(
                "Failed to initialize DDS subscriber for topic: {}",
                topic_name
            );
            return false;
        }

        subscriber->set_callback(
            [this, channel, converter = std::move(converter)](
                const Msg& msg)
            {
                push(channel, converter(msg));
            }
        );

        ros_connectables_.emplace_back(*subscriber);
        subscribers_.emplace_back(std::move(subscriber));
        return true;
    }

    void init_ros_graph_bridge(
        const RobotConfig& robot_config,
        dds::domain::DomainParticipant& participant
    );

    advrf::dds_common::ReaderPolicy reader_policy_;
    std::vector<std::shared_ptr<DDSSubscriberBase>> subscribers_;
    std::unique_ptr<CycloneDDSRosGraphBridge> ros_graph_bridge_;
    std::vector<std::reference_wrapper<IConnectRosGraphBridge>> ros_connectables_;
};