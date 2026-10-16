#pragma once

#include <deque>
#include <map>
#include <string>
#include <utility>

#include <advrf_cyclonedds_plugin/ros_metadata/ros_graph_bridge.hpp>
#include <advrf_cyclonedds_plugin/service/service_server.hpp>
#include <advrf_interfaces/msg/MotorControl.hpp>
#include <advrf_interfaces/srv/MotorControl.hpp>
#include <advrf_middleware_core/adapters_mode/motor_control.hpp>
#include <advrf_middleware_core/config/config_topics.hpp>

namespace advrf::cyclonedds_plugin
{

using MotorRequestDDS = advrf_interfaces::msg::dds_::MotorControlRequest_;
using RosMotorRequestDDS = advrf_interfaces::srv::dds_::MotorControl_Request_;
using RosMotorReplyDDS = advrf_interfaces::srv::dds_::MotorControl_Response_;
using MotorReplyDDS = advrf_interfaces::msg::dds_::MotorControlReply_;

/** Dedicated DDS request/reply adapter; motor commands use ZeroMQ, never SHM.
 * ROS-compatible service framing stays in this middleware adapter.
 */
class DDSAdapterMotorControl : public advrf::middleware::adapters_mode::MotorControlAdapter
{
public:
    DDSAdapterMotorControl(const advrf::middleware::config::ConfigTopics& topics,
        dds::domain::DomainParticipant& participant,
        advrf::middleware::adapters_mode::MotorControlConfig config);

    void spin_once() override;

private:
    MotorReplyDDS process(const MotorRequestDDS& request);

    ServiceServer<RosMotorRequestDDS, RosMotorReplyDDS> ros_server_;
    std::unique_ptr<CycloneDDSRosGraphBridge> ros_graph_;
    using Key = std::pair<std::string, uint64_t>;
    std::map<Key, MotorReplyDDS> replies_;
    std::deque<Key> order_;
};

} // namespace advrf::cyclonedds_plugin
