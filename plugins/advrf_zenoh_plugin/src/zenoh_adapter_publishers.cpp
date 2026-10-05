#include "advrf_zenoh_plugin/adapters/zenoh_adapter_publishers.hpp"
#include <utility>
#include <vector>

#include <advrf_middleware_core/utils/log.hpp>
#include "advrf_zenoh_plugin/adapters/zenoh_adapter_bridge.hpp"

namespace advrf::zenoh_plugin
{
namespace
{

using EcatId = advrf::middleware::pdo::EcatId;
using Ros2MessageType = serialization::Ros2MessageType;

} 

bool ZenohAdapterPublishers::init(const advrf::middleware::config::ConfigTopics& topics,
                                  const advrf::middleware::config::RobotConfig& robot,
                                  const advrf::middleware::ecat::EcatDiscover::EcatMap& ecat_map,
                                  zenoh::Session& session,
                                  WireFormat wire_format)
{
    auto register_topic = [this, &session, wire_format]
        (std::vector<advrf::middleware::shm::ChannelRx> channels,
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

    const auto filter_ids = [&ecat_map](advrf::middleware::shm::ChannelRx channel)
    {
        std::vector<EcatId> ids;
        ids.reserve(ecat_map.size());
        for (const auto& [id, device] : ecat_map)
        {
            if (device.channel == channel)
                ids.push_back(id);
        }
        return ids;
    };

    const auto motor_ids = filter_ids(advrf::middleware::shm::ChannelRx::Motor);
    const auto valve_ids = filter_ids(advrf::middleware::shm::ChannelRx::Valve);
    const auto gripper_ids = filter_ids(advrf::middleware::shm::ChannelRx::Gripper);

    auto joint_ids = motor_ids;
    joint_ids.insert(joint_ids.end(), valve_ids.begin(), valve_ids.end());
    joint_ids.insert(joint_ids.end(), gripper_ids.begin(), gripper_ids.end());

    bool success = true;
    success &= register_topic(
        {advrf::middleware::shm::ChannelRx::Motor, 
        advrf::middleware::shm::ChannelRx::Gripper, 
        advrf::middleware::shm::ChannelRx::Valve},
        joint_ids,
        topics.rx.jointState(),
        Ros2MessageType::JointState);
    success &= register_topic(
        {advrf::middleware::shm::ChannelRx::Motor}, motor_ids, topics.rx.motor(),
        Ros2MessageType::Motor);
    success &= register_topic(
        {advrf::middleware::shm::ChannelRx::Valve}, valve_ids, topics.rx.valve(),
        Ros2MessageType::Valve);
    success &= register_topic(
        {advrf::middleware::shm::ChannelRx::Gripper}, gripper_ids, topics.rx.gripper(),
        Ros2MessageType::Gripper);

    auto register_devices = [&register_topic, &robot, &filter_ids](
        advrf::middleware::shm::ChannelRx channel,
        const auto& make_key,
        Ros2MessageType ros2_message_type)
    {
        bool result = true;
        for (const auto id : filter_ids(channel))
        {
            const auto name = robot.map_ecat_id.find(id);
            if (name == robot.map_ecat_id.end())
            {
                LOG_ERROR("ECAT ID {} not found in robot configuration.", id);
                result = false;
                continue;
            }

            result &= register_topic(
                {channel},
                {id},
                make_key(name->second),
                ros2_message_type);
        }
        return result;
    };

    success &= register_devices(
        advrf::middleware::shm::ChannelRx::Imu,
        [&topics](const std::string& name) { return topics.rx.imu(name); },
        Ros2MessageType::Imu);
    success &= register_devices(
        advrf::middleware::shm::ChannelRx::PowerBoard,
        [&topics](const std::string& name) { return topics.rx.powerBoard(name); },
        Ros2MessageType::PowerBoard);
    success &= register_devices(
        advrf::middleware::shm::ChannelRx::Pump,
        [&topics](const std::string& name) { return topics.rx.pump(name); },
        Ros2MessageType::Pump);
    success &= register_devices(
        advrf::middleware::shm::ChannelRx::ForceTorque,
        [&topics](const std::string& name) { return topics.rx.forceTorque(name); },
        Ros2MessageType::ForceTorque);

    return success;
}

} 
