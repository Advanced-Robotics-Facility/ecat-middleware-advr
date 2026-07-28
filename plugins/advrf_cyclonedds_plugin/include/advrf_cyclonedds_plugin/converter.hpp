#pragma once
#include <cstdint>

// protobuf service
#include <advrf_interfaces_protobuf/repl_cmd.pb.h>
// protobuf messages
#include <advrf_interfaces_protobuf/ecat_pdo.pb.h>
// DDS service
#include <advrf_interfaces/srv/ReplCmd.hpp>
// DDS messages
#include <builtin_interfaces/msg/Time.hpp>
#include <std_msgs/msg/Header.hpp>
#include <sensor_msgs/msg/JointState.hpp>
#include <advrf_interfaces/msg/Motor.hpp>
#include <advrf_interfaces/msg/Imu.hpp>
#include <advrf_interfaces/msg/Valve.hpp>
#include <advrf_interfaces/msg/Pump.hpp>
#include <advrf_interfaces/msg/Gripper.hpp>
#include <advrf_interfaces/msg/PowerBoard.hpp>
#include <advrf_interfaces/msg/ForceTorque.hpp>
#include <advrf_interfaces/msg/ReplCmdContent.hpp>

namespace convert::protobuf {
    template<typename DDS_TYPE, typename PROTOBUF_TYPE>
    PROTOBUF_TYPE from_dds(const DDS_TYPE&) = delete;

    template<typename DDS_TYPE, typename PROTOBUF_TYPE>
    void from_dds(const DDS_TYPE&, PROTOBUF_TYPE&) = delete;


    void from_dds(const builtin_interfaces::msg::dds_::Time_& msgdds, iit::advrf::Time& pb);

    void from_dds(
        const std_msgs::msg::dds_::Header_& msgdds,
        iit::advrf::Header& pb);

    void from_dds(
        const advrf_interfaces::msg::dds_::Gains_& msgdds,
        iit::advrf::Gains& pb);

    void from_dds(
        const advrf_interfaces::msg::dds_::TrajectoryCmd_& msgdds,
        iit::advrf::Trajectory_cmd& pb);

    void from_dds(
        const advrf_interfaces::msg::dds_::CtrlCmd_& msgdds,
        iit::advrf::Ctrl_cmd& pb);

    void from_dds(
    const advrf_interfaces::msg::dds_::FlashCmd_& msgdds,
        iit::advrf::Flash_cmd& pb);

    void from_dds(
        const advrf_interfaces::msg::dds_::EcatMasterCmd_& msgdds,
        iit::advrf::Ecat_Master_cmd& pb);

     void from_dds(
        const advrf_interfaces::msg::dds_::FoeMaster_& msgdds,
        iit::advrf::FOE_Master& pb);

     void from_dds(
        const advrf_interfaces::msg::dds_::TrjQueueCmd_& msgdds,
        iit::advrf::Trj_queue_cmd& pb);

     void from_dds(
        const advrf_interfaces::msg::dds_::SlaveSdoCmd_& msgdds,
        iit::advrf::Slave_SDO_cmd& pb);

     void from_dds(
        const advrf_interfaces::msg::dds_::SlaveSdoInfo_& msgdds,
        iit::advrf::Slave_SDO_info& pb);

     void from_dds(
        const advrf_interfaces::msg::dds_::MotorsPdoCmd_& msgdds,
        iit::advrf::Motors_PDO_cmd& pb);

     void from_dds(
        const advrf_interfaces::msg::dds_::SlaveRegistryWrite_& msgdds,
        iit::advrf::Slave_registry_write& pb);

     void from_dds(
        const advrf_interfaces::msg::dds_::PdoAuxCmd_& msgdds,
        iit::advrf::PDOs_aux_cmd& pb);

    void from_dds(
        const advrf_interfaces::msg::dds_::ReplCmd_Content_& request,
        iit::advrf::Repl_cmd& pb);

     void from_dds(
        const advrf_interfaces::srv::dds_::ReplCmd_Request_& request,
        iit::advrf::Repl_cmd& pb);

      void from_dds(
        const advrf_interfaces::msg::dds_::ReplCmd_Content_Vector_& request,
        iit::advrf::Repl_cmd_vector& pb);
};

namespace convert::dds {
    template<typename DDS_TYPE, typename PROTOBUF_TYPE>
     void from_protobuf(const PROTOBUF_TYPE&, DDS_TYPE) = delete;
     void from_protobuf(uint64_t timestamp_ns, builtin_interfaces::msg::dds_::Time_& msgdds);
     void from_protobuf(const iit::advrf::Ec_slave_pdo& pb, std_msgs::msg::dds_::Header_& dds_time);
     void from_protobuf(const iit::advrf::Cmd_reply& pb, advrf_interfaces::srv::dds_::ReplCmd_Response_& dds_response);
     void from_protobuf(const iit::advrf::ImuVN_rx_pdo& pb, advrf_interfaces::msg::dds_::Imu_& ddsmsg);
     void from_protobuf(const iit::advrf::Cia402_rx_pdo& pb, sensor_msgs::msg::dds_::JointState_& ddsmsg);
     void from_protobuf(const iit::advrf::Cia402_rx_pdo& pb, advrf_interfaces::msg::dds_::Motor_& ddsmsg);
     void from_protobuf(const iit::advrf::Motor_xt_rx_pdo& pb, sensor_msgs::msg::dds_::JointState_& ddsmsg);
     void from_protobuf(const iit::advrf::Motor_xt_rx_pdo& pb, advrf_interfaces::msg::dds_::Motor_& ddsmsg);
     void from_protobuf(const iit::advrf::Motor_rx_pdo& pb, sensor_msgs::msg::dds_::JointState_& ddsmsg);
     void from_protobuf(const iit::advrf::Motor_rx_pdo& pb, advrf_interfaces::msg::dds_::Motor_& ddsmsg);
     void from_protobuf(const iit::advrf::HyqKnee_rx_pdo& pb, sensor_msgs::msg::dds_::JointState_& ddsmsg);
     void from_protobuf(const iit::advrf::HyqKnee_rx_pdo& pb, advrf_interfaces::msg::dds_::Valve_& ddsmsg);
     void from_protobuf(const iit::advrf::HyqHpu_rx_pdo& pb, advrf_interfaces::msg::dds_::Pump_& ddsmsg);
     void from_protobuf(const iit::advrf::Gripper_rx_pdo& pb, advrf_interfaces::msg::dds_::Gripper_& ddsmsg);
     void from_protobuf(const iit::advrf::Gripper_rx_pdo& pb, sensor_msgs::msg::dds_::JointState_ & ddsmsg);
     void from_protobuf(const iit::advrf::PowF28M36_rx_pdo& pb, advrf_interfaces::msg::dds_::PowerBoard_ & ddsmsg);
     void from_protobuf(const iit::advrf::FT6_rx_pdo& pb, advrf_interfaces::msg::dds_::ForceTorque_& ddsmsg);
};

namespace convert::shm {
    template<typename SHM_TYPE, typename PROTOBUF_TYPE>
    SHM_TYPE from_protobuf(const PROTOBUF_TYPE&) = delete;

    
};
