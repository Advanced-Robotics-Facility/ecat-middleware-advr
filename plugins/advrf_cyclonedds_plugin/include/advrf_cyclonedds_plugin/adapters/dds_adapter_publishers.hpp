#pragma once

#include <dds/dds.hpp>

#include <functional>
#include <initializer_list>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <advrf_middleware_core/adapters/adapter_publishers.hpp>
#include <advrf_middleware_core/config/config_topics.hpp>
#include <advrf_middleware_core/config/robot_config.hpp>
#include <advrf_middleware_core/utils/ecat_discover.hpp>

#include "advrf_cyclonedds_plugin/ros_metadata/ros_graph_bridge.hpp"

namespace advrf::cyclonedds_plugin {

using AdapterPublishers = middleware_adapter::message::AdapterPublishers;
using IPublisher = middleware_adapter::message::AdapterPublishers::IPublisher;
using Subscription = middleware_adapter::message::AdapterPublishers::Subscription;

/**
 * @brief Adapter that publishes received EtherCAT PDO data through DDS.
 *
 * It configures concrete publishers from robot metadata, maps EtherCAT IDs to
 * device names, and optionally exposes the publishers through the ROS graph.
 */
class DDSAdapterPublishers : public AdapterPublishers {
public:
    /**
     * @brief Configure all DDS publishers and ROS-graph integration.
     *
     * @param config_topics Topic-name builders.
     * @param robot_config Robot DDS and namespace configuration.
     * @param ecat_map Metadata discovered from EtherCAT PDOs.
     * @param dp DDS domain participant used to create entities.
     * @return True if initialization succeeds.
     */
    bool init(
        const config::ConfigTopics& config_topics,
        const config::RobotConfig& robot_config,
        const EcatDiscover::EcatMap& ecat_map,
        dds::domain::DomainParticipant& dp
    );

private:
    /**
     * @brief Create, register, and initialize a concrete PDO DDS publisher.
     *
     * The publisher is registered with the requested channels and EtherCAT-ID
     * filter, then made available to the ROS graph bridge.
     */
    template<typename Publisher, typename Topic>
    Publisher& create_publisher(
        std::initializer_list<ChannelRx> channels,
        const std::vector<pdo_utils::EcatId>& ids,
        const std::unordered_map<pdo_utils::EcatId, std::string>& names,
        const Topic& topic,
        dds::domain::DomainParticipant& dp
    )
    {
        auto& publisher = register_publisher<Publisher>(channels, ids);
        publisher.set_names(names);
        publisher.init(topic, dp);
        ros_connectables_.emplace_back(publisher);
        return publisher;
    }

    /// Initialize optional ROS 2 graph-discovery integration.
    void init_ros_graph_bridge(
        const config::RobotConfig& robot_config,
        dds::domain::DomainParticipant& dp
    );

    std::unique_ptr<CycloneDDSRosGraphBridge> ros_graph_bridge_;
    std::vector<std::reference_wrapper<IConnectRosGraphBridge>> ros_connectables_;
};

}