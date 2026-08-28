#include "advrf_zenoh_plugin/adapters/zenoh_adapter_subscribers.hpp"

#include <advrf_middleware_core/utils/log.hpp>

namespace advrf::zenoh_plugin
{

ZenohAdapterSubscribers::ZenohAdapterSubscribers(
        const config::ConfigTopics& topics,
        zenoh::Session& session,
        WireFormat wire_format)
    : session_(session)
    , wire_format_(wire_format)
    , topics_{
#ifdef ZENOH_ROS2_SUPPORT
        {topics.tx.jointCmd(), Pdo::TX_CIA402, ChannelTx::Motor,
            deserialization::Ros2CommandType::Joint},
        {topics.tx.motorCmd(), Pdo::TX_MOTOR, ChannelTx::Motor,
            deserialization::Ros2CommandType::Motor},
        {topics.tx.valveCmd(), Pdo::TX_HYQ_KNEE, ChannelTx::Valve,
            deserialization::Ros2CommandType::Valve},
        {topics.tx.gripperCmd(), Pdo::TX_GRIPPER, ChannelTx::Gripper,
            deserialization::Ros2CommandType::Gripper},
        {topics.tx.pumpCmd(), Pdo::TX_HYQ_HPU, ChannelTx::Pump,
            deserialization::Ros2CommandType::Pump},
        {topics.tx.powerBoardCmd(), Pdo::TX_POW_F28M36, ChannelTx::PowerBoard,
            deserialization::Ros2CommandType::PowerBoard},
        {topics.tx.forceTorqueCmd(), Pdo::TX_FT6, ChannelTx::ForceTorque,
            deserialization::Ros2CommandType::ForceTorque},
#else
        {topics.tx.jointCmd(), Pdo::TX_CIA402, ChannelTx::Motor},
        {topics.tx.motorCmd(), Pdo::TX_MOTOR, ChannelTx::Motor},
        {topics.tx.valveCmd(), Pdo::TX_HYQ_KNEE, ChannelTx::Valve},
        {topics.tx.gripperCmd(), Pdo::TX_GRIPPER, ChannelTx::Gripper},
        {topics.tx.pumpCmd(), Pdo::TX_HYQ_HPU, ChannelTx::Pump},
        {topics.tx.powerBoardCmd(), Pdo::TX_POW_F28M36, ChannelTx::PowerBoard},
        {topics.tx.forceTorqueCmd(), Pdo::TX_FT6, ChannelTx::ForceTorque},
#endif
    }
{}

bool ZenohAdapterSubscribers::start()
{
    if (!middleware_adapter::message::AdapterSubscribers::start())
        return false;

    subscribers_.clear();
    subscribers_.reserve(topics_.size());

    for (const auto& topic : topics_)
    {
        bool registered = false;
        if (wire_format_ == WireFormat::Protobuf)
            registered = register_subscriber(topic);
#ifdef ZENOH_ROS2_SUPPORT
        else
            registered = register_cdr_subscriber(topic);
#endif

        if (!registered)
        {
            subscribers_.clear();
            return false;
        }
    }

    return true;
}

bool ZenohAdapterSubscribers::register_subscriber(const Topic& topic)
{
    try
    {
        subscribers_.emplace_back(std::make_unique<ZenohSubscriber<Pdo>>(
            session_,
            topic.key,
            [this, topic](const Pdo& pdo)
            {
                enqueue(topic, pdo);
            }));

        LOG_INFO("Protobuf Subscriber Created: {}", topic.key);
        return true;
    }
    catch (const zenoh::ZException& error)
    {
        LOG_ERROR("Failed to declare Zenoh subscriber '{}': {}", topic.key, error.what());
        return false;
    }
}

#ifdef ZENOH_ROS2_SUPPORT
bool ZenohAdapterSubscribers::register_cdr_subscriber(const Topic& topic)
{
    try
    {
        using Commands = std::vector<Pdo>;
        const deserialization::Ros2CdrDeserializer decoder(topic.cdr_type);

        subscribers_.emplace_back(std::make_unique<ZenohSubscriber<Commands>>(
            session_, topic.key,
            [this, topic](const Commands& commands)
            {
                for (const auto& pdo : commands)
                    if (!enqueue(topic, pdo))
                        break;
            },
            [decoder](const std::vector<std::uint8_t>& payload,
                      Commands& commands)
            {
                return decoder.deserialize_cycle(payload, commands);
            }));

        LOG_INFO("ROS 2 CDR command subscriber created: {}", topic.key);
        return true;
    }
    catch (const zenoh::ZException& error)
    {
        LOG_ERROR("Failed to declare Zenoh subscriber '{}': {}", topic.key, error.what());
        return false;
    }
}
#endif

bool ZenohAdapterSubscribers::enqueue(const Topic& topic, const Pdo& pdo)
{
    if (pdo.type() != topic.expected_type)
    {
        LOG_ERROR(
            "Rejected command with PDO type {} on Zenoh key '{}'; expected {}.",
            static_cast<int>(pdo.type()), topic.key,
            static_cast<int>(topic.expected_type));
        return false;
    }

    if (push(topic.channel, pdo))
        return true;

    LOG_ERROR(
        "Failed to enqueue command from Zenoh key '{}'; the TX queue may be full.",
        topic.key);
    return false;
}

void ZenohAdapterSubscribers::spin_once()
{
    for (const auto& subscriber : subscribers_)
        subscriber->spin_once();
}

}
