#pragma once

#include <cstdint>
#include <mutex>
#include <stdexcept>
#include <string>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

#include <advrf_dds_common/ros_metadata/ros_graph_state.hpp>
#include <rmw_dds_common/msg/ParticipantEntitiesInfoPubSubTypes.hpp>

namespace advrf::fastdds_plugin {

class FastRosGraphBridge : public RosGraphState {
public:
  using Participant = eprosima::fastdds::dds::DomainParticipant;
  using DataWriter = eprosima::fastdds::dds::DataWriter;
  using DataReader = eprosima::fastdds::dds::DataReader;
  using Publisher = eprosima::fastdds::dds::Publisher;
  using Topic = eprosima::fastdds::dds::Topic;
  using TypeSupport = eprosima::fastdds::dds::TypeSupport;

  static std::string build_node_namespace(const std::string &ns,
                                          const std::string &robot_name) {
    return "/" + ns + "/fastdds/" + robot_name;
  }

  FastRosGraphBridge(Participant *participant, std::string node_name,
                     std::string node_namespace)
      : participant_(participant), node_name_(std::move(node_name)),
        node_namespace_(std::move(node_namespace)),
        type_(new rmw_dds_common::msg::dds_::
                  ParticipantEntitiesInfo_PubSubType()) {
    if (!participant_) {
      throw std::runtime_error("FastRosGraphBridge: null participant");
    }

    participant_gid_ = guid_to_gid(participant_->guid());

    init();
  }

  ~FastRosGraphBridge() { cleanup(); }

  void add_writer(DataWriter *writer) {
    if (!writer) {
      return;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    add_writer_gid(guid_to_gid(writer->guid()));
    publish_locked();
  }

  void remove_writer(DataWriter *writer) {
    if (!writer) {
      return;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    remove_writer_gid(guid_to_gid(writer->guid()));
    publish_locked();
  }

  void add_reader(DataReader *reader) {
    if (!reader) {
      return;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    add_reader_gid(guid_to_gid(reader->guid()));
    publish_locked();
  }

  void remove_reader(DataReader *reader) {
    if (!reader) {
      return;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    remove_reader_gid(guid_to_gid(reader->guid()));
    publish_locked();
  }

  void publish() {
    std::lock_guard<std::mutex> lock(mutex_);
    publish_locked();
  }

private:
  template <typename GuidT> static GidBytes guid_to_gid(const GuidT &guid) {
    GidBytes result{};
    std::copy_n(guid.guidPrefix.value, 12, result.begin());
    std::copy_n(guid.entityId.value, 4, result.begin() + 12);
    return result;
  }

  void init() {
    using namespace eprosima::fastdds::dds;

    if (type_.register_type(participant_) != RETCODE_OK) {
      throw std::runtime_error("Failed to register ros_discovery_info type");
    }

    topic_ = participant_->create_topic(
        "ros_discovery_info", type_.get_type_name(), TOPIC_QOS_DEFAULT);

    if (!topic_) {
      throw std::runtime_error("Failed to create ros_discovery_info topic");
    }

    publisher_ = participant_->create_publisher(PUBLISHER_QOS_DEFAULT, nullptr);

    if (!publisher_) {
      throw std::runtime_error("Failed to create ROS graph publisher");
    }

    auto qos = publisher_->get_default_datawriter_qos();
    qos.reliability().kind = RELIABLE_RELIABILITY_QOS;
    qos.durability().kind = TRANSIENT_LOCAL_DURABILITY_QOS;
    qos.history().kind = KEEP_LAST_HISTORY_QOS;
    qos.history().depth = 1;

    writer_ = publisher_->create_datawriter(topic_, qos, nullptr);

    if (!writer_) {
      throw std::runtime_error("Failed to create ros_discovery_info writer");
    }

    publish();
  }

  // ROS Humble Gid = 24 bytes
  static rmw_dds_common::msg::dds_::Gid_ make_ros_gid(const GidBytes &gid) {
    rmw_dds_common::msg::dds_::Gid_ result;
    auto &data = result.data();
    std::fill(data.begin(), data.end(), uint8_t{0});
    std::copy(gid.begin(), gid.end(), data.begin());
    return result;
  }

  void publish_locked() {
    using ParticipantInfo = rmw_dds_common::msg::dds_::ParticipantEntitiesInfo_;
    using NodeInfo = rmw_dds_common::msg::dds_::NodeEntitiesInfo_;
    ParticipantInfo msg;
    msg.gid(make_ros_gid(participant_gid_));
    NodeInfo node;
    node.node_name(node_name_);
    node.node_namespace(node_namespace_);

    for (const auto &gid : writer_gids()) {
      node.writer_gid_seq().push_back(make_ros_gid(gid));
    }

    for (const auto &gid : reader_gids()) {
      node.reader_gid_seq().push_back(make_ros_gid(gid));
    }

    msg.node_entities_info_seq().push_back(std::move(node));
    const auto ret = writer_->write(&msg);
    if (ret != eprosima::fastdds::dds::RETCODE_OK) {
      throw std::runtime_error("Failed to publish ros_discovery_info");
    }
  }

  void cleanup() {
    if (publisher_ && writer_) {
      publisher_->delete_datawriter(writer_);
      writer_ = nullptr;
    }

    if (participant_ && publisher_) {
      participant_->delete_publisher(publisher_);
      publisher_ = nullptr;
    }

    if (participant_ && topic_) {
      participant_->delete_topic(topic_);
      topic_ = nullptr;
    }
  }

private:
  Participant *participant_{nullptr};
  Publisher *publisher_{nullptr};
  Topic *topic_{nullptr};
  DataWriter *writer_{nullptr};

  TypeSupport type_;
  std::string node_name_;
  std::string node_namespace_;

  GidBytes participant_gid_{};

  std::mutex mutex_;
};

class IConnectRosGraphBridge {
public:
  virtual ~IConnectRosGraphBridge() = default;
  virtual void connect_ros_graph_bridge(FastRosGraphBridge &bridge) = 0;
};

}