#pragma once

#include <cstdint>
#include <utility>
#include <variant>
#include <vector>
#include <stdexcept>
#include <type_traits>

#include "advrf_zenoh_plugin/config/wire_format.hpp"
#include "advrf_zenoh_plugin/serialization/protobuf.hpp"
#include "advrf_zenoh_plugin/serialization/ros2cdr.hpp"

namespace advrf::zenoh_plugin::serialization {

class Serializer {
public:
    explicit Serializer(WireFormat wire_format) {
        switch (wire_format) {
            case WireFormat::Protobuf:
                serializer_.emplace<ProtobufSerializer>();
                break;
            case WireFormat::Ros2Cdr:
#ifdef ZENOH_ROS2_SUPPORT
                serializer_.emplace<Ros2CdrSerializer>();
#else
            throw std::runtime_error("ROS 2 CDR support was not compiled in.");
#endif
                break;
        }
    }

    template <typename Message>
    bool serialize(const Message& message,
                   std::vector<std::uint8_t>& payload) const
    {
        return std::visit(
            [&](const auto& serializer)
            {
                return serializer.serialize(
                    message,
                    payload
                );
            },
            serializer_
        );
    }

    bool serialize_cycle(const std::vector<iit::advrf::Ec_slave_pdo>& messages,
                         std::vector<std::uint8_t>& payload) const
    {
        return std::visit(
            [&](const auto& serializer)
            {
                using SerializerType = std::decay_t<decltype(serializer)>;
                if constexpr (std::is_same_v<SerializerType, Ros2CdrSerializer>)
                    return serializer.serialize(messages, payload);

                payload.clear();
                return false;
            },
            serializer_
        );
    }

private:
    std::variant<ProtobufSerializer, Ros2CdrSerializer> serializer_;
};

}
