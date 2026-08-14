#include "advrf_zenoh_plugin/serialization/ros2cdr.hpp"
#include "advrf_zenoh_plugin/serialization/cdr_utils.hpp"

#include <cstdint>
#include <string>

#include <advrf_dds_common/converter/converter.hpp>
#include <advrf_interfaces/msg/Imu.hpp>
#include <advrf_interfaces/msg/ImuCdrAux.hpp>
#include <advrf_interfaces/msg/Motor.hpp>
#include <advrf_interfaces/msg/MotorCdrAux.hpp>

namespace advrf::zenoh_plugin::serialization::ros2cdr
{
namespace
{

struct MotorSample
{
    const iit::advrf::Header* header;
    const iit::advrf::Cia402_rx_pdo* motor;
};

bool serialize_motors(const std::vector<MotorSample>& samples,
                      std::vector<std::uint8_t>& payload)
{
    payload.clear();
    if (samples.empty()) return false;

    for (const auto& sample : samples)
    {
        if (sample.header == nullptr || 
            sample.motor == nullptr ||
            !sample.header->has_stamp()
        )
            return false;
    }

    advrf_interfaces::msg::dds_::Motor_ message;

    for (const auto& sample : samples)
    {
        convert::dds::from_protobuf(*sample.motor, message);
        message.name().push_back(sample.header->str_id());
    }

    const auto& stamp = samples.back().header->stamp();
    message.header().stamp().sec() = stamp.sec();
    message.header().stamp().nanosec() = stamp.nsec();
    message.header().frame_id().clear();

    return write_cdr(payload, [&](auto& cdr)
    {
        eprosima::fastcdr::serialize(cdr, message);
    });
}

} 

bool serialize(const iit::advrf::Header& header,
               const iit::advrf::ImuVN_rx_pdo& imu,
               std::vector<std::uint8_t>& payload)
{
    advrf_interfaces::msg::dds_::Imu_ message;
    convert::dds::from_protobuf(imu, message);

    const auto& stamp = header.stamp();
    message.header().stamp().sec() = stamp.sec();
    message.header().stamp().nanosec() = stamp.nsec();
    message.header().frame_id() = header.str_id();

    return write_cdr(payload, [&](auto& cdr)
    {
        eprosima::fastcdr::serialize(cdr, message);
    });
}

bool serialize(const iit::advrf::Header& header,
               const iit::advrf::Cia402_rx_pdo& motor,
               std::vector<std::uint8_t>& payload)
{
    return serialize_motors({MotorSample{&header, &motor}}, payload);
}

bool serialize(const std::vector<iit::advrf::Ec_slave_pdo>& pdos,
               std::vector<std::uint8_t>& payload)
{
    std::vector<MotorSample> samples;
    samples.reserve(pdos.size());

    for (const auto& pdo : pdos)
    {
        if (pdo.type() != iit::advrf::Ec_slave_pdo::RX_CIA402)
        {
            payload.clear();
            return false;
        }

        samples.push_back(MotorSample{&pdo.header(), &pdo.cia402_rx_pdo()});
    }

    return serialize_motors(samples, payload);
}

} 
