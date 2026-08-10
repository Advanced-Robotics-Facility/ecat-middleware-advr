#include "advrf_zenoh_plugin/adapters/zenoh_adapter_publishers.hpp"
#include <advrf_middleware_core/utils/log.hpp>
#include "advrf_zenoh_plugin/adapters/zenoh_adapter_bridge.hpp"

namespace advrf::zenoh_plugin
{
namespace
{

using Adapter = ZenohAdapterPublishers;
using EcatId = Adapter::EcatId;

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
    if (wire_format == WireFormat::Ros2Cdr)
    {
        LOG_ERROR("ROS 2 CDR support was selected, but its publishers are not implemented yet.");
        return false;
    }

    auto register_topic = [this, &session](std::vector<Channel> channels,
                                           std::vector<EcatId> ids,
                                           const std::string& key)
    {
        if (ids.empty())
            return true;

        auto& publisher = register_publisher<ZenohAdapterBridgePublisher>(std::move(channels), ids);
        return publisher.init(session, key);
    };

    const auto joint_ids = extract_ids(robot.joints);
    const auto motor_ids = extract_ids(robot.motors);
    const auto valve_ids = extract_ids(robot.valves);
    const auto gripper_ids = extract_ids(robot.grippers);

    bool success = true;
    success &= register_topic(
        {ChannelToShm::Motor, ChannelToShm::Gripper, ChannelToShm::Valve},
        joint_ids,
        topics.state.jointState());
    success &= register_topic(
        {ChannelToShm::Motor}, motor_ids, topics.state.motor());
    success &= register_topic(
        {ChannelToShm::Valve}, valve_ids, topics.state.valve());
    success &= register_topic(
        {ChannelToShm::Gripper}, gripper_ids, topics.state.gripper());

    auto register_devices = [&register_topic](
        const std::vector<JointConfig>& devices,
        Channel channel,
        const auto& make_key)
    {
        bool result = true;
        for (const auto& device : devices)
        {
            if (device.ecat_id < 0)
                continue;

            result &= register_topic(
                {channel},
                {static_cast<EcatId>(device.ecat_id)},
                make_key(device.name));
        }
        return result;
    };

    success &= register_devices(
        robot.imus,
        ChannelToShm::Imu,
        [&topics](const std::string& name) { return topics.state.imu(name); });
    success &= register_devices(
        robot.power_boards,
        ChannelToShm::PowerBoard,
        [&topics](const std::string& name) {
            return topics.state.powerBoard(name);
        });
    success &= register_devices(
        robot.pumps,
        ChannelToShm::Pump,
        [&topics](const std::string& name) { return topics.state.pump(name); });
    success &= register_devices(
        robot.force_torques,
        ChannelToShm::ForceTorque,
        [&topics](const std::string& name) {
            return topics.state.forceTorque(name);
        });

    return success;
}

} 
