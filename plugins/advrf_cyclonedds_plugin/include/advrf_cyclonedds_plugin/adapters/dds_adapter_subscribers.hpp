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

namespace advrf::cyclonedds_plugin {

/**
 * @brief DDS subscriber that can be registered with the ROS graph bridge.
 *
 * @tparam Msg DDS message type read from the topic.
 */
template <typename Msg>
class DDSPdoSubscriber
    : public IConnectRosGraphBridge,
      public DDSSubscriber<Msg>
{
public:
    /// Register this subscriber's DDS reader with the ROS graph bridge.
    void connect_ros_graph_bridge(CycloneDDSRosGraphBridge& bridge) override
    {
        bridge.add_reader(this->dds_reader());
    }
};

/**
 * @brief Adapter that forwards DDS command messages to EtherCAT shared memory.
 *
 * Each configured DDS topic is converted to one or more transmit PDOs and
 * pushed to its associated @c ChannelTx channel.
 */
class DDSAdapterSubscribers
    : public advrf::middleware::adapters::message::AdapterSubscribers
{
public:

    /**
     * @brief Configure command-topic subscribers and ROS graph integration.
     *
     * @param config_topics Topic-name builders.
     * @param robot_config Robot DDS configuration.
     * @param ecat_map Metadata from EtherCAT PDO discovery.
     * @param participant DDS participant used to create readers.
     * @param reader_policy QoS policy applied to every reader.
     * @return True if every required subscriber is initialized.
     */
    bool init(
        const advrf::middleware::config::ConfigTopics& config_topics,
        const advrf::middleware::config::RobotConfig& robot_config,
        const advrf::middleware::ecat::EcatDiscover::EcatMap &ecat_map,
        dds::domain::DomainParticipant& participant,
        advrf::dds_common::ReaderPolicy reader_policy = {}
    );

    /// Process all available samples from registered DDS readers.
    void spin_once() override;

private:
    /**
     * @brief Register a DDS topic whose message converts to multiple PDOs.
     *
     * @return False if the DDS reader cannot be initialized.
     */
    template <typename Msg>
    bool register_subscriber(
        const std::string& topic_name,
        dds::domain::DomainParticipant& participant,
        advrf::middleware::shm::ChannelTx channel,
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

    /**
     * @brief Register a DDS topic whose message converts to one PDO.
     *
     * @return False if the DDS reader cannot be initialized.
     */
    template <typename Msg>
    bool register_subscriber(
        const std::string& topic_name,
        dds::domain::DomainParticipant& participant,
        advrf::middleware::shm::ChannelTx channel,
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

    /// Initialize optional ROS 2 graph-discovery integration.
    void init_ros_graph_bridge(
        const advrf::middleware::config::RobotConfig& robot_config,
        dds::domain::DomainParticipant& participant
    );

    advrf::dds_common::ReaderPolicy reader_policy_;
    std::vector<std::shared_ptr<DDSSubscriberBase>> subscribers_;
    std::unique_ptr<CycloneDDSRosGraphBridge> ros_graph_bridge_;
    std::vector<std::reference_wrapper<IConnectRosGraphBridge>> ros_connectables_;
};

}