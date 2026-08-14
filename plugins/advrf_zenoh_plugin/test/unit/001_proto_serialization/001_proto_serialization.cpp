#include <cassert>
#include <cstdint>
#include <vector>

#include <advrf_interfaces_protobuf/ecat_pdo.pb.h>

#include "advrf_zenoh_plugin/serialization/protobuf.hpp"

int main()
{
    using advrf::zenoh_plugin::protobuf::deserialize;
    using advrf::zenoh_plugin::protobuf::serialize;

    iit::advrf::Ec_slave_pdo source;
    source.set_type(iit::advrf::Ec_slave_pdo::RX_IMU_VN);
    source.mutable_header()->set_str_id("imu_7");
    source.mutable_header()->set_index(42);
    source.mutable_imuvn_rx_pdo()->set_x_acc(1.25);

    std::vector<std::uint8_t> payload;
    assert(serialize(source, payload));
    assert(!payload.empty());

    iit::advrf::Ec_slave_pdo decoded;
    assert(deserialize(payload, decoded));
    assert(decoded.type() == source.type());
    assert(decoded.header().str_id() == "imu_7");
    assert(decoded.header().index() == 42);
    assert(decoded.imuvn_rx_pdo().x_acc() == 1.25);

    const std::vector<std::uint8_t> invalid{0xff, 0xff, 0xff};
    iit::advrf::Ec_slave_pdo rejected;
    assert(!deserialize(invalid, rejected));

    return 0;
}
