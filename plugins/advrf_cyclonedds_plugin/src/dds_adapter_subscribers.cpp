#include "advrf_cyclonedds_plugin/adapters/dds_adapter_subscribers.hpp"
#include <advrf_interfaces/msg/MotorTxPdo.hpp>

DDSAdapterSubscribers::DDSAdapterSubscribers(
    const config::ConfigTopics &config_topics, const RobotConfig &robot_config,
    dds::domain::DomainParticipant &participant,
    advrf::dds_common::ReaderPolicy reader_policy)
    : reader_policy_(std::move(reader_policy)) {

 register_subscriber<advrf_interfaces::msg::dds_::MotorTxPdoVector_>(
    config_topics.tx.motors(),
    participant,
    ChannelTx::Motor,
    [this](const advrf_interfaces::msg::dds_::MotorTxPdoVector_& msg) {
      std::vector<iit::advrf::Ec_slave_pdo> result;
      result.reserve(msg.data().size());

      for (const auto& element : msg.data()) {
        auto& pdo = result.emplace_back();
        const auto type = resolve_type(element.ecat_id());
        convert::protobuf::from_dds(element, type, pdo);
      }

      return result;
    });

  init_ros_graph_bridge(robot_config, participant);
}

void DDSAdapterSubscribers::init_ros_graph_bridge(
    const RobotConfig &robot_config,
    dds::domain::DomainParticipant &participant) {
    if (!robot_config.declare_to_ros) {
        return;
    }

  const auto node_namespace = CycloneDDSRosGraphBridge::build_node_namespace(
      robot_config.ns, robot_config.robot_name);

  ros_graph_bridge_ =
      std::make_unique<CycloneDDSRosGraphBridge>(participant, "tx_node", node_namespace);

  for (auto &connectable : ros_connectables_) {
    connectable.get().connect_ros_graph_bridge(*ros_graph_bridge_);
  }
}

void DDSAdapterSubscribers::spin_once() {
  for (const auto &subscriber : subscribers_) {
    subscriber->spin_once();
  }
}