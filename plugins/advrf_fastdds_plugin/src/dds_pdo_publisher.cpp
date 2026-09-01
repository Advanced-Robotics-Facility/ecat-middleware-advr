#include "advrf_fastdds_plugin/adapters/dds_pdo_publisher.hpp"
#include "advrf_middleware_core/utils/name_resolver.hpp"

namespace advrf::fastdds_plugin {

bool ImuPublisher::process(const iit::advrf::Ec_slave_pdo& pdo) {
    switch (pdo.type()) {
        case iit::advrf::Ec_slave_pdo::RX_IMU_VN:
            advrf::dds_common::convert::dds::from_protobuf(pdo.imuvn_rx_pdo(), message());
            break;
        default:
            LOG_WARN("Unexpected PDO type for ImuPublisher: {}", static_cast<int>(pdo.type()));
            return false;
    }

    advrf::dds_common::convert::dds::from_protobuf(pdo, message().header());
    message().header().frame_id() = resolve_name(pdo.header().str_id(), id_to_name_);
    return true;
}

bool JointStatePublisher::process(const iit::advrf::Ec_slave_pdo& pdo) {
    switch (pdo.type()) {
        case iit::advrf::Ec_slave_pdo::RX_CIA402:
            advrf::dds_common::convert::dds::from_protobuf(pdo.cia402_rx_pdo(), message());
            break;
        case iit::advrf::Ec_slave_pdo::RX_XT_MOTOR:
            advrf::dds_common::convert::dds::from_protobuf(pdo.motor_xt_rx_pdo(), message());
            break;
        case iit::advrf::Ec_slave_pdo::RX_MOTOR:
            advrf::dds_common::convert::dds::from_protobuf(pdo.motor_rx_pdo(), message());
            break;
        case iit::advrf::Ec_slave_pdo::RX_GRIPPER:
            advrf::dds_common::convert::dds::from_protobuf(pdo.gripper_rx_pdo(), message());
            break;
        default:
            LOG_WARN("Unexpected PDO type for JointStatePublisher: {}", static_cast<int>(pdo.type()));
            return false; // Exit early if the PDO type is not handled
    }
    
    message().name().push_back(resolve_name(pdo.header().str_id(), id_to_name_));
    advrf::dds_common::convert::dds::from_protobuf(pdo, message().header());
    return true;
}

bool MotorsPublisher::process(const iit::advrf::Ec_slave_pdo& pdo) {
    switch (pdo.type()) {
        case iit::advrf::Ec_slave_pdo::RX_CIA402:
            advrf::dds_common::convert::dds::from_protobuf(pdo.cia402_rx_pdo(), message());
            break;
        case iit::advrf::Ec_slave_pdo::RX_XT_MOTOR:
            advrf::dds_common::convert::dds::from_protobuf(pdo.motor_xt_rx_pdo(), message());
            break;
        case iit::advrf::Ec_slave_pdo::RX_MOTOR:
            advrf::dds_common::convert::dds::from_protobuf(pdo.motor_rx_pdo(), message());
            break;
        default:
            LOG_WARN("Unexpected PDO type for MotorsPublisher: {}", static_cast<int>(pdo.type()));
            return false; // Exit early if the PDO type is not handled
    }
    
    message().name().push_back(resolve_name(pdo.header().str_id(), id_to_name_));
    advrf::dds_common::convert::dds::from_protobuf(pdo, message().header());
    return true;
}

bool PowerBoardPublisher::process(const iit::advrf::Ec_slave_pdo& pdo) {
    switch (pdo.type()) {
        case iit::advrf::Ec_slave_pdo::RX_POW_F28M36:
            advrf::dds_common::convert::dds::from_protobuf(pdo.powf28m36_rx_pdo(), message());
            break;
        default:
            LOG_WARN("Unexpected PDO type for PowerBoardPublisher: {}", static_cast<int>(pdo.type()));
            return false; // Exit early if the PDO type is not handled
    }
    
    advrf::dds_common::convert::dds::from_protobuf(pdo, message().header());
    message().header().frame_id() = resolve_name(pdo.header().str_id(), id_to_name_);
    return true;
}

bool PumpPublisher::process(const iit::advrf::Ec_slave_pdo& pdo) {
    switch (pdo.type()) {
        case iit::advrf::Ec_slave_pdo::RX_HYQ_HPU:
            advrf::dds_common::convert::dds::from_protobuf(pdo.hyqhpu_rx_pdo(), message());
            break;
        default:
            LOG_WARN("Unexpected PDO type for PumpPublisher: {}", static_cast<int>(pdo.type()));
            return false; // Exit early if the PDO type is not handled
    }
    
    advrf::dds_common::convert::dds::from_protobuf(pdo, message().header());
    message().header().frame_id() = resolve_name(pdo.header().str_id(), id_to_name_);
    return true;
}

bool ForceTorquePublisher::process(const iit::advrf::Ec_slave_pdo& pdo) {
    switch (pdo.type()) {
        case iit::advrf::Ec_slave_pdo::RX_FT6:
            advrf::dds_common::convert::dds::from_protobuf(pdo.ft6_rx_pdo(), message());
            break;
        default:
            LOG_WARN("Unexpected PDO type for ForceTorquePublisher: {}", static_cast<int>(pdo.type()));
            return false; // Exit early if the PDO type is not handled
    }
    
    advrf::dds_common::convert::dds::from_protobuf(pdo, message().header());
    message().header().frame_id() = resolve_name(pdo.header().str_id(), id_to_name_);
    return true;
}

bool ValvePublisher::process(const iit::advrf::Ec_slave_pdo& pdo) {
    switch (pdo.type()) {
        case iit::advrf::Ec_slave_pdo::RX_HYQ_KNEE:
            advrf::dds_common::convert::dds::from_protobuf(pdo.hyqknee_rx_pdo(), message());
            break;
        default:
            LOG_WARN("Unexpected PDO type for ValvePublisher: {}", static_cast<int>(pdo.type()));
            return false; // Exit early if the PDO type is not handled
    }
    
    message().name().push_back(resolve_name(pdo.header().str_id(), id_to_name_));
    advrf::dds_common::convert::dds::from_protobuf(pdo, message().header());
    return true;
}

bool GripperPublisher::process(const iit::advrf::Ec_slave_pdo& pdo) {
    switch (pdo.type()) {
        case iit::advrf::Ec_slave_pdo::RX_GRIPPER:
            advrf::dds_common::convert::dds::from_protobuf(pdo.gripper_rx_pdo(), message());
            break;
        default:
            LOG_WARN("Unexpected PDO type for GripperPublisher: {}", static_cast<int>(pdo.type()));
            return false; // Exit early if the PDO type is not handled
    }
    
    message().name().push_back(resolve_name(pdo.header().str_id(), id_to_name_));
    advrf::dds_common::convert::dds::from_protobuf(pdo, message().header());
    return true;
}

}