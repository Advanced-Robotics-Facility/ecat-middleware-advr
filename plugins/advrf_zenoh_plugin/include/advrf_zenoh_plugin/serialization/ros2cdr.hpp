#pragma once

#include <cstdint>
#include <vector>

#include <advrf_interfaces_protobuf/ecat_pdo.pb.h>
#include <advrf_middleware_core/utils/log.hpp>

namespace advrf::zenoh_plugin::serialization::ros2cdr
{

#ifdef ZENOH_ROS2_SUPPORT

bool serialize(const iit::advrf::Header& header,
               const iit::advrf::ImuVN_rx_pdo& imu,
               std::vector<std::uint8_t>& payload);

bool serialize(const iit::advrf::Header& header,
               const iit::advrf::Cia402_rx_pdo& motor,  // Motor_rx_pdo, Motor_xt_rx_pdo
               std::vector<std::uint8_t>& payload);

bool serialize(const std::vector<iit::advrf::Ec_slave_pdo>& pdos,
               std::vector<std::uint8_t>& payload);
        
#endif

// template<typename ProtobufType>
// bool deserialize(const zenoh::Bytes&,
//                  iit::advrf::Header&,
//                  ProtobufType&) = delete;

// bool deserialize(const zenoh::Bytes& payload,
//                  iit::advrf::Header& header,
//                  iit::advrf::ImuVN_rx_pdo& imu);

} 

namespace advrf::zenoh_plugin::serialization 
{
class Ros2CdrSerializer {
public:
    bool serialize(const iit::advrf::Ec_slave_pdo& pdo,
                   std::vector<std::uint8_t>& payload) const
    {
#ifdef ZENOH_ROS2_SUPPORT
        switch(pdo.type()) 
        {
            case iit::advrf::Ec_slave_pdo::RX_IMU_VN:
                return ros2cdr::serialize(
                    pdo.header(),
                    pdo.imuvn_rx_pdo(),
                    payload
                );
            
            case iit::advrf::Ec_slave_pdo::RX_CIA402:
                return ros2cdr::serialize(
                    pdo.header(),
                    pdo.cia402_rx_pdo(),
                    payload
                );

            default:
                LOG_ERROR(
                    "ROS2 CDR serialization not implemented for PDO type {}",
                    static_cast<int>(pdo.type())
                );
                return false;
        }
#else
        (void)pdo;
        (void)payload;
        return false;
#endif
    }

    bool serialize(const std::vector<iit::advrf::Ec_slave_pdo>& pdos,
                   std::vector<std::uint8_t>& payload) const
    {
#ifdef ZENOH_ROS2_SUPPORT
        if (pdos.size() == 1 &&
            pdos.front().type() == iit::advrf::Ec_slave_pdo::RX_IMU_VN)
        {
            return serialize(pdos.front(), payload);
        }

        return ros2cdr::serialize(pdos, payload);
#else
        (void)pdos;
        (void)payload;
        return false;
#endif
    }
};

}
