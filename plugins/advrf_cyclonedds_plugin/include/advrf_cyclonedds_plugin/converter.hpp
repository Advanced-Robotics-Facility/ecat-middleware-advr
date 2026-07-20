#pragma once
#include <cstdint>
#include <string>

#include <advrf_interfaces_protobuf/repl_cmd.pb.h>
#include <advrf_interfaces/srv/ReplCmd.hpp>
#include "advrf_cyclonedds_plugin/service/shm_data.hpp"

namespace convert::protobuf {
    template<typename DDS_TYPE, typename PROTOBUF_TYPE>
    PROTOBUF_TYPE from_dds(const DDS_TYPE&) = delete;

    inline iit::advrf::Time from_dds(const builtin_interfaces::msg::dds_::Time_& msgdds)
    {
        iit::advrf::Time pb_time;
        pb_time.set_sec(msgdds.sec());
        pb_time.set_nsec(msgdds.nanosec());
        return pb_time;
    }

    inline iit::advrf::Header from_dds(const std_msgs::msg::dds_::Header_& msgdds)
    {
        iit::advrf::Header pb_header;
        pb_header.set_str_id(static_cast<std::string>(msgdds.frame_id()));
        pb_header.mutable_stamp()->set_sec(msgdds.stamp().sec());
        pb_header.mutable_stamp()->set_nsec(msgdds.stamp().nanosec());
        return pb_header;
    }

    inline iit::advrf::Gains from_dds(const advrf_interfaces::msg::dds_::Gains_& msgdds)
    {
        iit::advrf::Gains pb;
        pb.set_type(static_cast<iit::advrf::Gains::Type>(msgdds.type()));
        pb.set_pos_kp(msgdds.pos_kp());
        pb.set_pos_kd(msgdds.pos_kd());
        pb.set_tor_kp(msgdds.tor_kp());
        pb.set_tor_ki(msgdds.tor_ki());
        pb.set_tor_kd(msgdds.tor_kd());
        return pb;
    }

    inline iit::advrf::Trajectory_cmd from_dds(const advrf_interfaces::msg::dds_::TrajectoryCmd_& msgdds)
    {
        iit::advrf::Trajectory_cmd pb_trj_cmd;
        pb_trj_cmd.set_type(static_cast<iit::advrf::Trajectory_cmd::Type>(msgdds.type()));
        pb_trj_cmd.set_name(msgdds.name());
        pb_trj_cmd.set_board_id(msgdds.board_id());


        auto pb_smooth_par = pb_trj_cmd.mutable_smooth_par();
        for (const auto& x : msgdds.smooth_par().x()) {
            pb_smooth_par->add_x(x);
        }
        for (const auto& y : msgdds.smooth_par().y()) {
            pb_smooth_par->add_y(y);
        }

        auto pb_period_par = pb_trj_cmd.mutable_period_par();
        pb_period_par->set_freq(msgdds.period_par().freq());
        pb_period_par->set_ampl(msgdds.period_par().ampl());
        pb_period_par->set_teta(msgdds.period_par().teta());
        pb_period_par->set_secs(msgdds.period_par().secs());

        auto pb_homing_par = pb_trj_cmd.mutable_homing_par();
        for (const auto& x : msgdds.homing_par().x()) {
            pb_homing_par->add_x(x);
        }
        
        auto pb_smooth_vel_par = pb_trj_cmd.mutable_smooth_vel_par();
        pb_smooth_vel_par->set_dt(msgdds.smooth_vel_par().dt());
        pb_smooth_vel_par->set_p0(msgdds.smooth_vel_par().p0());
        pb_smooth_vel_par->set_v0(msgdds.smooth_vel_par().v0());
        pb_smooth_vel_par->set_pt(msgdds.smooth_vel_par().pt());
        pb_smooth_vel_par->set_vt(msgdds.smooth_vel_par().vt());
        pb_smooth_vel_par->set_ds(msgdds.smooth_vel_par().ds());
        pb_smooth_vel_par->set_magic(msgdds.smooth_vel_par().magic());

        return pb_trj_cmd;
    }

    inline iit::advrf::Ctrl_cmd from_dds(const advrf_interfaces::msg::dds_::CtrlCmd_& msgdds)
    {
        iit::advrf::Ctrl_cmd pb_ctrl_cmd;
        pb_ctrl_cmd.set_type(static_cast<iit::advrf::Ctrl_cmd::Type>(msgdds.type()));
        pb_ctrl_cmd.set_board_id(msgdds.board_id());
        pb_ctrl_cmd.set_value(msgdds.value());
        *pb_ctrl_cmd.mutable_gains() = from_dds(msgdds.gains());
        return pb_ctrl_cmd;
    }

    inline iit::advrf::Flash_cmd from_dds(const advrf_interfaces::msg::dds_::FlashCmd_& msgdds)
    {
        iit::advrf::Flash_cmd pb_flash_cmd;
        pb_flash_cmd.set_type(static_cast<iit::advrf::Flash_cmd::Type>(msgdds.type()));
        pb_flash_cmd.set_board_id(msgdds.board_id());
        return pb_flash_cmd;
    }

    inline iit::advrf::Ecat_Master_cmd from_dds(const advrf_interfaces::msg::dds_::EcatMasterCmd_& msgdds)
    {
        iit::advrf::Ecat_Master_cmd pb_ecat_master_cmd;
        pb_ecat_master_cmd.set_type(static_cast<iit::advrf::Ecat_Master_cmd::Type>(msgdds.type()));
        auto pb_args = pb_ecat_master_cmd.mutable_args();
        for (const auto& arg : msgdds.args()) {
            auto pb_arg = pb_args->Add();
            pb_arg->set_name(arg.name());
            pb_arg->set_value(arg.value());
        }
        return pb_ecat_master_cmd;
    }

    inline iit::advrf::FOE_Master from_dds(const advrf_interfaces::msg::dds_::FoeMaster_& msgdds)
    {
        iit::advrf::FOE_Master pb_foe_master;
        pb_foe_master.set_filename(msgdds.filename());
        pb_foe_master.set_password(msgdds.password());
        pb_foe_master.set_mcu_type(msgdds.mcu_type());
        pb_foe_master.set_slave_pos(msgdds.slave_pos());
        pb_foe_master.set_board_id(msgdds.board_id());
        return pb_foe_master;
    }   


    inline iit::advrf::Trj_queue_cmd from_dds(const advrf_interfaces::msg::dds_::TrjQueueCmd_& msgdds)
    {
        iit::advrf::Trj_queue_cmd pb;
        pb.set_type(static_cast<iit::advrf::Trj_queue_cmd::Type>(msgdds.type()));
        for (const auto& name : msgdds.trj_names()) {
            pb.add_trj_names(name);
        }
        return pb;
    }

    inline iit::advrf::Slave_SDO_cmd from_dds(const advrf_interfaces::msg::dds_::SlaveSdoCmd_& msgdds)
    {
        iit::advrf::Slave_SDO_cmd pb;
        pb.set_board_id(msgdds.board_id());
        for(const auto& rd_sdo : msgdds.rd_sdo()) {
            pb.add_rd_sdo(rd_sdo);
        }
        for(const auto& wr_sdo : msgdds.wr_sdo()) {
            auto pb_wr_sdo = pb.add_wr_sdo();
            pb_wr_sdo->set_name(wr_sdo.name());
            pb_wr_sdo->set_value(wr_sdo.value());
        }
        return pb;
    }

    inline iit::advrf::Slave_SDO_info from_dds(const advrf_interfaces::msg::dds_::SlaveSdoInfo_& msgdds)
    {
        iit::advrf::Slave_SDO_info pb;
        pb.set_type(static_cast<iit::advrf::Slave_SDO_info::Type>(msgdds.type()));
        pb.set_board_id(msgdds.board_id());
        return pb;
    }

 

    inline iit::advrf::Motors_PDO_cmd from_dds(const advrf_interfaces::msg::dds_::MotorsPdoCmd_& msgdds)
    {
        iit::advrf::Motors_PDO_cmd pb;
        for(const auto& motor: msgdds.motors_pdo()) {
            auto pb_motor = pb.add_motors_pdo();
            pb_motor->set_motor_id(motor.motor_id());
            pb_motor->set_pos_ref(motor.pos_ref());
            pb_motor->set_vel_ref(motor.vel_ref());
            pb_motor->set_tor_ref(motor.tor_ref());
            *pb_motor->mutable_gains() = from_dds(motor.gains());
        }
        return pb;
    }

    inline iit::advrf::Slave_registry_write from_dds(const advrf_interfaces::msg::dds_::SlaveRegistryWrite_& msgdds)
    {
        iit::advrf::Slave_registry_write pb;
        pb.set_type(static_cast<iit::advrf::Slave_registry_write::Type>(msgdds.type()));
        pb.set_board_id(msgdds.board_id());
        return pb;
    }

    inline iit::advrf::PDOs_aux_cmd from_dds(const advrf_interfaces::msg::dds_::PdoAuxCmd_& msgdds)
    {
        iit::advrf::PDOs_aux_cmd pb;
        for(const auto& aux_cmd: msgdds.aux_cmds()) {
            auto pb_aux_cmd = pb.add_aux_cmds();
            pb_aux_cmd->set_type(static_cast<iit::advrf::PDOs_aux_cmd::Aux_cmd::Type>(aux_cmd.type()));
            pb_aux_cmd->set_board_id(aux_cmd.board_id());
        }
        return pb;
    }

    inline iit::advrf::Request_header from_dds(const rcl_interfaces::msg::dds_::RequestHeader_& msgdds)
    {
        iit::advrf::Request_header pb;
        pb.set_guid(msgdds.guid());
        pb.set_seq(msgdds.seq());
        return pb;
    }

    inline iit::advrf::Repl_cmd from_dds(const advrf_interfaces::srv::dds_::ReplCmd_Request_& request)
    {
        iit::advrf::Repl_cmd pb_repl_cmd;
        pb_repl_cmd.set_type(static_cast<iit::advrf::CmdType>(request.type()));
        
        *pb_repl_cmd.mutable_trajectory_cmd() = from_dds(request.trajectory_cmd());
        *pb_repl_cmd.mutable_ctrl_cmd() = from_dds(request.ctrl_cmd());
        *pb_repl_cmd.mutable_flash_cmd() = from_dds(request.flash_cmd());
        *pb_repl_cmd.mutable_trajectory_cmd() = from_dds(request.trajectory_cmd());
        *pb_repl_cmd.mutable_ecat_master_cmd() = from_dds(request.ecat_master_cmd());
        *pb_repl_cmd.mutable_foe_master() = from_dds(request.foe_master());
        *pb_repl_cmd.mutable_trj_queue_cmd() = from_dds(request.trj_queue_cmd());
        *pb_repl_cmd.mutable_slave_sdo_cmd() = from_dds(request.slave_sdo_cmd());
        *pb_repl_cmd.mutable_slave_sdo_info() = from_dds(request.slave_sdo_info());
        *pb_repl_cmd.mutable_motors_pdo_cmd() = from_dds(request.motors_pdo_cmd());
        *pb_repl_cmd.mutable_slave_registry_write() = from_dds(request.slave_registry_write());
        *pb_repl_cmd.mutable_pdos_aux_cmd() = from_dds(request.pdos_aux_cmd());

        return pb_repl_cmd;
    }
}

namespace convert::dds {
    template<typename DDS_TYPE, typename PROTOBUF_TYPE>
    DDS_TYPE from_protobuf(const PROTOBUF_TYPE&) = delete;
    
     
    inline advrf_interfaces::srv::dds_::ReplCmd_Response_ from_protobuf(const iit::advrf::Cmd_reply& pb)
    {
        advrf_interfaces::srv::dds_::ReplCmd_Response_ reply;
        reply.type() = static_cast<uint8_t>(pb.type());
        reply.msg() = pb.msg();
        reply.header().frame_id() = pb.header().str_id();
        reply.header().stamp().sec() = pb.header().stamp().sec();
        reply.header().stamp().nanosec() = pb.header().stamp().nsec();
        reply.pdo() = pb.pdo();
        return reply;
    }
};