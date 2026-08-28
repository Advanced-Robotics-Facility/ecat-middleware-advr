#pragma once

#include <cstdint>
#include <limits>
#include <vector>

namespace advrf::zenoh_plugin::serialization
{

class ProtobufSerializer
{
public:
    template<typename Message>
    bool serialize(const Message& message, std::vector<std::uint8_t>& payload) const
    {
        const auto size = message.ByteSizeLong();
        if (size > static_cast<std::size_t>(std::numeric_limits<int>::max()))
            return false;

        payload.resize(size);
        return message.SerializeToArray(payload.data(), static_cast<int>(size));
    }
};

}

namespace advrf::zenoh_plugin::deserialization
{

class ProtobufDeserializer
{
public:
    template<typename Message>
    bool deserialize(const std::vector<std::uint8_t>& payload,
                     Message& message) const
    {
        if (payload.size() > static_cast<std::size_t>(std::numeric_limits<int>::max()))
            return false;

        return message.ParseFromArray(payload.data(), static_cast<int>(payload.size()));
    }
};

}
