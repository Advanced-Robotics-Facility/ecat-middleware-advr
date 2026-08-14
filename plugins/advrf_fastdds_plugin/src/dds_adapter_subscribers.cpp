#include "advrf_fastdds_plugin/adapters/dds_adapter_subscribers.hpp"
#include <advrf_interfaces/msg/MotorXtTxPdo.hpp>
#include <advrf_interfaces/msg/MotorTxPdo.hpp>
#include <advrf_interfaces/msg/MotorXtTxPdoPubSubTypes.hpp>
#include <advrf_interfaces/msg/MotorTxPdoPubSubTypes.hpp>

DDSAdapterSubscribers::DDSAdapterSubscribers(const config::ConfigTopics& config_topics,
                                             const RobotConfig& robot_config,
                                             eprosima::fastdds::dds::DomainParticipant* participant,
                                             advrf::dds_common::ReaderPolicy reader_policy)
    : reader_policy_(reader_policy) {

        register_subscriber<
            advrf_interfaces::msg::dds_::MotorXtTxPdoVector_,
            advrf_interfaces::msg::dds_::MotorXtTxPdoVector_PubSubType
        >(
            config_topics.tx.motorXtCmd(), participant, 
            ChannelTx::Motor, [](const advrf_interfaces::msg::dds_::MotorXtTxPdoVector_& msg) {
                return convert::protobuf::vector_from_dds(msg);
            });

        register_subscriber<
            advrf_interfaces::msg::dds_::MotorTxPdoVector_,
            advrf_interfaces::msg::dds_::MotorTxPdoVector_PubSubType
        >(
            config_topics.tx.motorCmd(), participant, 
            ChannelTx::Motor, [](const advrf_interfaces::msg::dds_::MotorTxPdoVector_& msg) {
                return convert::protobuf::vector_from_dds(msg);
            });

        init_ros_graph_bridge(robot_config, participant);
    }


void DDSAdapterSubscribers::init_ros_graph_bridge(
    const RobotConfig &robot_config,
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