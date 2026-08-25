#pragma once

#include <memory>
#include <string>
#include <vector>

#include <zenoh.hxx>

#include <advrf_interfaces_protobuf/ecat_pdo.pb.h>
#include <advrf_middleware_core/adapters/adapter_subscribers.hpp>
#include <advrf_middleware_core/config/config_topics.hpp>

#include "advrf_zenoh_plugin/subscriber/zenoh_subscriber.hpp"
#include "advrf_zenoh_plugin/config/wire_format.hpp"
#ifdef ZENOH_ROS2_SUPPORT
#include "advrf_zenoh_plugin/serialization/ros2cdr.hpp"
#endif

namespace advrf::zenoh_plugin
{

class ZenohAdapterSubscribers
    : public middleware_adapter::message::AdapterSubscribers
{
public:
    ZenohAdapterSubscribers(const config::ConfigTopics& topics,
                            zenoh::Session& session,
                            WireFormat wire_format);

    bool start() override;
    void spin_once() override;

private:
    using Pdo = iit::advrf::Ec_slave_pdo;

    struct Topic
    {
        std::string key;
        Pdo::Type expected_type;
        ChannelTx channel;
#ifdef ZENOH_ROS2_SUPPORT
        deserialization::Ros2CommandType cdr_type;
#endif
    };

    bool register_subscriber(const Topic& topic);
#ifdef ZENOH_ROS2_SUPPORT
    bool register_cdr_subscriber(const Topic& topic);
#endif
    bool enqueue(const Topic& topic, const Pdo& pdo);

    zenoh::Session& session_;
    WireFormat wire_format_;
    std::vector<Topic> topics_;
    std::vector<std::unique_ptr<IZenohSubscriber>> subscribers_;
};

}
