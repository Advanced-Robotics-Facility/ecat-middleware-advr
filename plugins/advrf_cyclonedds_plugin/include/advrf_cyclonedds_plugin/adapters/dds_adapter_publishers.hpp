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

using AdapterPublishers = middleware_adapter::message::AdapterPublishers;
using IPublisher = middleware_adapter::message::AdapterPublishers::IPublisher;
using Subscription = middleware_adapter::message::AdapterPublishers::Subscription;

class DDSAdapterPublishers : public AdapterPublishers {
public:
    bool init(
        const config::ConfigTopics& config_topics,
        const RobotConfig& robot_config,
        const EcatDiscover::EcatMap& ecat_map,
        dds::domain::DomainParticipant& dp
    );

private:
    template<typename Publisher, typename Topic>
    Publisher& create_publisher(
        std::initializer_list<ChannelRx> channels,
        const std::vector<EcatId>& ids,
        const std::unordered_map<EcatId, std::string>& names,
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

    void init_ros_graph_bridge(
        const RobotConfig& robot_config,
        dds::domain::DomainParticipant& dp
    );

    std::unique_ptr<CycloneDDSRosGraphBridge> ros_graph_bridge_;
    std::vector<std::reference_wrapper<IConnectRosGraphBridge>> ros_connectables_;
};