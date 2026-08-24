#pragma once

#include <cstdint>
#include <vector>

#include <fastcdr/Cdr.h>
#include <fastcdr/FastBuffer.h>
#include <fastcdr/exceptions/Exception.h>

namespace advrf::zenoh_plugin::serialization::ros2cdr
{

template<typename Message>
bool serialize_idl(const Message& message, std::vector<std::uint8_t>& payload)
{
    payload.clear();

    try
    {
        eprosima::fastcdr::FastBuffer buffer;
        eprosima::fastcdr::Cdr cdr(
            buffer,
            eprosima::fastcdr::Cdr::DEFAULT_ENDIAN,
            eprosima::fastcdr::CdrVersion::XCDRv1);

        cdr.serialize_encapsulation();
        eprosima::fastcdr::serialize(cdr, message);

        const auto size = cdr.get_serialized_data_length();
        const auto* data = reinterpret_cast<const std::uint8_t*>(buffer.getBuffer());
        payload.assign(data, data + size);
        return true;
    }
    catch (const eprosima::fastcdr::exception::Exception&)
    {
        payload.clear();
        return false;
    }
}

}
