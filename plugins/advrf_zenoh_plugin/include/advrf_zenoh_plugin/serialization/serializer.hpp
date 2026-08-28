#pragma once

#include <cstdint>
#include <stdexcept>
#include <vector>

#include "advrf_zenoh_plugin/config/wire_format.hpp"
#include "advrf_zenoh_plugin/serialization/protobuf.hpp"
#include "advrf_zenoh_plugin/serialization/ros2cdr.hpp"

namespace advrf::zenoh_plugin::serialization
{

class Serializer
{
public:
    Serializer(WireFormat wire_format, Ros2MessageType ros2_message_type)
        : wire_format_(wire_format)
        , ros2_(ros2_message_type)
    {
#ifndef ZENOH_ROS2_SUPPORT
        if (wire_format_ == WireFormat::Ros2Cdr)
            throw std::runtime_error("ROS 2 CDR support was not compiled in.");
#endif
    }

    template<typename Message>
    bool serialize(const Message& message,
                   std::vector<std::uint8_t>& payload) const
    {
        if (wire_format_ != WireFormat::Protobuf)
        {
            payload.clear();
            return false;
        }

        return protobuf_.serialize(message, payload);
    }

    bool serialize_cycle(
        const std::vector<iit::advrf::Ec_slave_pdo>& messages,
        std::vector<std::uint8_t>& payload) const
    {
#ifdef ZENOH_ROS2_SUPPORT
        if (wire_format_ == WireFormat::Ros2Cdr)
            return ros2_.serialize_cycle(messages, payload);
#else
        (void)messages;
#endif

        payload.clear();
        return false;
    }

private:
    WireFormat wire_format_;
    ProtobufSerializer protobuf_;
    Ros2CdrSerializer ros2_;
};

}
