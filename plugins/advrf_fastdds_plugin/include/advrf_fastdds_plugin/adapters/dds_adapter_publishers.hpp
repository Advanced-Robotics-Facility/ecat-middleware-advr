#pragma once

#include <fastdds/dds/domain/DomainParticipant.hpp>

#include <advrf_middleware_core/adapters/adapter_publishers.hpp>
#include <advrf_middleware_core/config/config_topics.hpp>
#include <advrf_middleware_core/config/robot_config.hpp>
#include <advrf_middleware_core/utils/ecat_discover.hpp>
#include <advrf_fastdds_plugin/ros_metadata/ros_graph_bridge.hpp>  

namespace advrf::fastdds_plugin {

using AdapterPublishers = middleware_adapter::message::AdapterPublishers;
using IPublisher = middleware_adapter::message::AdapterPublishers::IPublisher;
using Subscription = middleware_adapter::message::AdapterPublishers::Subscription;

class DDSAdapterPublishers : public AdapterPublishers {
public:
    bool init(
        const config::ConfigTopics& config_topics,
        const config::RobotConfig& robot_config,
        const EcatDiscover::EcatMap& ecat_map,
        eprosima::fastdds::dds::DomainParticipant* dp
    );

private:
    template<typename Publisher, typename Topic>
    Publisher& create_publisher(
        std::initializer_list<ChannelRx> channels,
        const std::vector<pdo_utils::EcatId>& ids,
        const std::unordered_map<pdo_utils::EcatId, std::string>& names,
        const Topic& topic,
        eprosima::fastdds::dds::DomainParticipant* dp
    )
    {
        auto& publisher = register_publisher<Publisher>(channels, ids);
        publisher.set_names(names);
        publisher.init(topic, dp);
        ros_connectables_.emplace_back(publisher);
        return publisher;
    }

    void init_ros_graph_bridge(
        const config::RobotConfig& robot_config,
        eprosima::fastdds::dds::DomainParticipant* dp
    );

    std::unique_ptr<FastRosGraphBridge> ros_graph_bridge_;
    std::vector<std::reference_wrapper<IConnectRosGraphBridge>> ros_connectables_;
};

}