#include "advrf_fastdds_plugin/adapters/dds_adapter_subscribers.hpp"
#include <advrf_interfaces/msg/MotorTxPdo.hpp>
#include <advrf_interfaces/msg/MotorTxPdoPubSubTypes.hpp>

namespace advrf::fastdds_plugin {

bool DDSAdapterSubscribers::init(const advrf::middleware::config::ConfigTopics& config_topics,
                                  const advrf::middleware::config::RobotConfig& robot_config,
                                  const advrf::middleware::ecat::EcatDiscover::EcatMap& ecat_map,
                                  eprosima::fastdds::dds::DomainParticipant* participant,
                                  advrf::dds_common::ReaderPolicy reader_policy)
    {

    reader_policy_ = std::move(reader_policy);
     register_subscriber<
            advrf_interfaces::msg::dds_::MotorTxPdoVector_,
            advrf_interfaces::msg::dds_::MotorTxPdoVector_PubSubType
        >(
            config_topics.tx.motorCmd(), participant, 
            advrf::middleware::shm::ChannelTx::Motor,  [&](const advrf_interfaces::msg::dds_::MotorTxPdoVector_& msg) {
            std::vector<iit::advrf::Ec_slave_pdo> result;
            result.reserve(msg.data().size());

            for (const auto& element : msg.data()) {
                auto& pdo = result.emplace_back();
                const auto type = resolve_type(ecat_map, element.ecat_id());
                advrf::dds_common::convert::protobuf::from_dds(element, type, pdo);
            }

            return result;
        });

        init_ros_graph_bridge(robot_config, participant);
        return true;
    }


void DDSAdapterSubscribers::init_ros_graph_bridge(
    const advrf::middleware::config::RobotConfig &robot_config,
    eprosima::fastdds::dds::DomainParticipant *participant) {
    if (!robot_config.declare_to_ros) {
        return;
    }

  const auto node_namespace = FastRosGraphBridge::build_node_namespace(
      robot_config.ns, robot_config.robot_name);

  ros_graph_bridge_ = std::make_unique<FastRosGraphBridge>(participant, "tx_node", node_namespace);

  for (auto &connectable : ros_connectables_) {
    connectable.get().connect_ros_graph_bridge(*ros_graph_bridge_);
  }
}

void DDSAdapterSubscribers::spin_once() {
  for (const auto &subscriber : subscribers_) {
    subscriber->spin_once();
  }
}

}