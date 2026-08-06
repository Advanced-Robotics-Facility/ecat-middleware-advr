#pragma once

#include <cstdint>
#include <vector>

#include <zenoh.hxx>

#include <advrf_interfaces_protobuf/ecat_pdo.pb.h>

namespace advrf::zenoh_plugin::ros2_cdr
{

template<typename ProtobufType>
bool serialize(const iit::advrf::Header&,
               const ProtobufType&,
               std::vector<std::uint8_t>&) = delete;

bool serialize(const iit::advrf::Header& header,
               const iit::advrf::ImuVN_rx_pdo& imu,
               std::vector<std::uint8_t>& payload);

template<typename ProtobufType>
bool deserialize(const zenoh::Bytes&,
                 iit::advrf::Header&,
                 ProtobufType&) = delete;

bool deserialize(const zenoh::Bytes& payload,
                 iit::advrf::Header& header,
                 iit::advrf::ImuVN_rx_pdo& imu);

} 
