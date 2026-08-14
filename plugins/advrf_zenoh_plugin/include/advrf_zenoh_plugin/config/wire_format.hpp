#pragma once

#include <zenoh.hxx>
namespace advrf::zenoh_plugin
{

enum class WireFormat
{
    Protobuf,
    Ros2Cdr,
};

inline zenoh::Encoding encoding_for(WireFormat wire_format)
{
    switch (wire_format)
    {
        case WireFormat::Protobuf:
            return zenoh::Encoding::Predefined::application_protobuf();

        case WireFormat::Ros2Cdr:
            return zenoh::Encoding::Predefined::application_octet_stream();
    }

    return zenoh::Encoding::Predefined::application_octet_stream();
}

} 
