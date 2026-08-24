#include "advrf_zenoh_plugin/adapters/zenoh_adapter_publishers.hpp"
#include <utility>
#include <vector>

#include <advrf_middleware_core/utils/log.hpp>
#include "advrf_zenoh_plugin/adapters/zenoh_adapter_bridge.hpp"

namespace advrf::zenoh_plugin
{
namespace
{

using Adapter = ZenohAdapterPublishers;
using EcatId = Adapter::EcatId;
using Ros2MessageType = serialization::Ros2MessageType;

std::vector<EcatId> extract_ids(const std::vector<JointConfig>& entries)
{
    std::vector<EcatId> ids;
    ids.reserve(entries.size());

    for (const auto& entry : entries)
    {
        if (entry.ecat_id < 0)
        {
            LOG_ERROR("Negative ecat_id {} in config, skipping.", entry.ecat_id);
            continue;
        }

        ids.push_back(static_cast<EcatId>(entry.ecat_id));
    }

    return ids;
}

} 

bool ZenohAdapterPublishers::init(const config::ConfigTopics& topics,
                                  const RobotConfig& robot,
                                  zenoh::Session& session,
                                  WireFormat wire_format)
{
    auto register_topic = [this, &session, wire_format]
        (std::vector<ChannelRx> channels,
        std::vector<EcatId> ids,
        const std::string& key,
        Ros2MessageType ros2_message_type)
    {
        if (ids.empty())
            return true;

        auto& publisher = register_publisher<ZenohAdapterBridgePublisher>(
            std::move(channels), ids);
        return publisher.init(session, key, wire_format, ros2_message_type);
    };

    const auto joint_ids = extract_ids(robot.joints);
    const auto motor_ids = extract_ids(robot.motors);
    const auto valve_ids = extract_ids(robot.valves);
    const auto gripper_ids = extract_ids(robot.grippers);

    bool success = true;
    success &= register_topic(
        {ChannelRx::Motor, ChannelRx::Gripper, ChannelRx::Valve},
        joint_ids,
        topics.state.jointState(),
        Ros2MessageType::JointState);
    success &= register_topic(
        {ChannelRx::Motor}, motor_ids, topics.state.motor(),
        Ros2MessageType::Motor);
    success &= register_topic(
        {ChannelRx::Valve}, valve_ids, topics.state.valve(),
        Ros2MessageType::Valve);
    success &= register_topic(
        {ChannelRx::Gripper}, gripper_ids, topics.state.gripper(),
        Ros2MessageType::Gripper);

    auto register_devices = [&register_topic](
        const std::vector<JointConfig>& devices,
        ChannelRx channel,
        const auto& make_key,
        Ros2MessageType ros2_message_type)
    {
        bool result = true;
        for (const auto& device : devices)
        {
            if (device.ecat_id < 0)
                continue;

            result &= register_topic(
                {channel},
                {static_cast<EcatId>(device.ecat_id)},
                make_key(device.name),
                ros2_message_type);
        }
        return result;
    };

    success &= register_devices(
        robot.imus,
        ChannelRx::Imu,
        [&topics](const std::string& name) { return topics.state.imu(name); },
        Ros2MessageType::Imu);
    success &= register_devices(
        robot.power_boards,
        ChannelRx::PowerBoard,
        [&topics](const std::string& name) { return topics.state.powerBoard(name); },
        Ros2MessageType::PowerBoard);
    success &= register_devices(
        robot.pumps,
        ChannelRx::Pump,
        [&topics](const std::string& name) { return topics.state.pump(name); },
        Ros2MessageType::Pump);
    success &= register_devices(
        robot.force_torques,
        ChannelRx::ForceTorque,
        [&topics](const std::string& name) { return topics.state.forceTorque(name); },
        Ros2MessageType::ForceTorque);

    return success;
}

} 
