#include "advrf_cyclonedds_plugin/converter.hpp"

void convert::protobuf::from_dds(const builtin_interfaces::msg::dds_::Time_& msgdds, iit::advrf::Time& pb)
{
    pb.set_sec(msgdds.sec());
    pb.set_nsec(msgdds.nanosec());
}

void convert::protobuf::from_dds(
        const std_msgs::msg::dds_::Header_& msgdds,
        iit::advrf::Header& pb)
{
    pb.set_str_id(static_cast<std::string>(msgdds.frame_id()));

    auto* stamp = pb.mutable_stamp();
    stamp->set_sec(msgdds.stamp().sec());
    stamp->set_nsec(msgdds.stamp().nanosec());
}

void convert::protobuf::from_dds(
        const advrf_interfaces::msg::dds_::Gains_& msgdds,
        iit::advrf::Gains& pb)
{
    pb.set_type(static_cast<iit::advrf::Gains::Type>(msgdds.type()));
    pb.set_pos_kp(msgdds.pos_kp());
    pb.set_pos_kd(msgdds.pos_kd());
    pb.set_tor_kp(msgdds.tor_kp());
    pb.set_tor_ki(msgdds.tor_ki());
    pb.set_tor_kd(msgdds.tor_kd());
}

void convert::protobuf::from_dds(
        const advrf_interfaces::msg::dds_::TrajectoryCmd_& msgdds,
        iit::advrf::Trajectory_cmd& pb)
{
    pb.set_type(static_cast<iit::advrf::Trajectory_cmd::Type>(msgdds.type()));
    pb.set_name(msgdds.name());
    pb.set_board_id(msgdds.board_id());

    auto* pb_smooth_par = pb.mutable_smooth_par();
    for (const auto& x : msgdds.smooth_par().x()) {
        pb_smooth_par->add_x(x);
    }
    for (const auto& y : msgdds.smooth_par().y()) {
        pb_smooth_par->add_y(y);
    }

    auto* pb_period_par = pb.mutable_period_par();
    pb_period_par->set_freq(msgdds.period_par().freq());
    pb_period_par->set_ampl(msgdds.period_par().ampl());
    pb_period_par->set_teta(msgdds.period_par().teta());
    pb_period_par->set_secs(msgdds.period_par().secs());

    auto* pb_homing_par = pb.mutable_homing_par();
    for (const auto& x : msgdds.homing_par().x()) {
        pb_homing_par->add_x(x);
    }

    auto* pb_smooth_vel_par = pb.mutable_smooth_vel_par();
    pb_smooth_vel_par->set_dt(msgdds.smooth_vel_par().dt());
    pb_smooth_vel_par->set_p0(msgdds.smooth_vel_par().p0());
    pb_smooth_vel_par->set_v0(msgdds.smooth_vel_par().v0());
    pb_smooth_vel_par->set_pt(msgdds.smooth_vel_par().pt());
    pb_smooth_vel_par->set_vt(msgdds.smooth_vel_par().vt());
    pb_smooth_vel_par->set_ds(msgdds.smooth_vel_par().ds());
    pb_smooth_vel_par->set_magic(msgdds.smooth_vel_par().magic());
}

void convert::protobuf::from_dds(
        const advrf_interfaces::msg::dds_::CtrlCmd_& msgdds,
        iit::advrf::Ctrl_cmd& pb)
{
    pb.set_type(static_cast<iit::advrf::Ctrl_cmd::Type>(msgdds.type()));
    pb.set_board_id(msgdds.board_id());
    pb.set_value(msgdds.value());

    from_dds(msgdds.gains(), *pb.mutable_gains());
}

void convert::protobuf::from_dds(
    const advrf_interfaces::msg::dds_::FlashCmd_& msgdds,
        iit::advrf::Flash_cmd& pb)
{
    pb.set_type(static_cast<iit::advrf::Flash_cmd::Type>(msgdds.type()));
    pb.set_board_id(msgdds.board_id());
}

void convert::protobuf::from_dds(
        const advrf_interfaces::msg::dds_::EcatMasterCmd_& msgdds,
        iit::advrf::Ecat_Master_cmd& pb)
{
    pb.set_type(static_cast<iit::advrf::Ecat_Master_cmd::Type>(msgdds.type()));

    auto* pb_args = pb.mutable_args();
    for (const auto& arg : msgdds.args()) {
        auto* pb_arg = pb_args->Add();
        pb_arg->set_name(arg.name());
        pb_arg->set_value(arg.value());
    }
}

void convert::protobuf::from_dds(
        const advrf_interfaces::msg::dds_::FoeMaster_& msgdds,
        iit::advrf::FOE_Master& pb)
    {
   pb.set_filename(msgdds.filename());
   pb.set_password(msgdds.password());
   pb.set_mcu_type(msgdds.mcu_type());
   pb.set_slave_pos(msgdds.slave_pos());
   pb.set_board_id(msgdds.board_id());
    }

void convert::protobuf::from_dds(
        const advrf_interfaces::msg::dds_::TrjQueueCmd_& msgdds,
        iit::advrf::Trj_queue_cmd& pb)
    {
   pb.set_type(static_cast<iit::advrf::Trj_queue_cmd::Type>(msgdds.type()));

   for (const auto& name : msgdds.trj_names()) {
       pb.add_trj_names(name);
   }
    }

void convert::protobuf::from_dds(
        const advrf_interfaces::msg::dds_::SlaveSdoCmd_& msgdds,
        iit::advrf::Slave_SDO_cmd& pb)
    {
   pb.set_board_id(msgdds.board_id());

   for (const auto& rd_sdo : msgdds.rd_sdo()) {
       pb.add_rd_sdo(rd_sdo);
   }

   for (const auto& wr_sdo : msgdds.wr_sdo()) {
       auto* pb_wr_sdo = pb.add_wr_sdo();
       pb_wr_sdo->set_name(wr_sdo.name());
       pb_wr_sdo->set_value(wr_sdo.value());
   }
    }

void convert::protobuf::from_dds(
        const advrf_interfaces::msg::dds_::SlaveSdoInfo_& msgdds,
        iit::advrf::Slave_SDO_info& pb)
    {
   pb.set_type(static_cast<iit::advrf::Slave_SDO_info::Type>(msgdds.type()));
   pb.set_board_id(msgdds.board_id());
    }

void convert::protobuf::from_dds(
        const advrf_interfaces::msg::dds_::MotorsPdoCmd_& msgdds,
        iit::advrf::Motors_PDO_cmd& pb)
    {
   for (const auto& motor : msgdds.motors_pdo()) {
       auto* pb_motor = pb.add_motors_pdo();
       pb_motor->set_motor_id(motor.motor_id());
       pb_motor->set_pos_ref(motor.pos_ref());
       pb_motor->set_vel_ref(motor.vel_ref());
       pb_motor->set_tor_ref(motor.tor_ref());

       from_dds(motor.gains(), *pb_motor->mutable_gains());
   }
    }

void convert::protobuf::from_dds(
        const advrf_interfaces::msg::dds_::SlaveRegistryWrite_& msgdds,
        iit::advrf::Slave_registry_write& pb)
    {
   pb.set_type(static_cast<iit::advrf::Slave_registry_write::Type>(msgdds.type()));
   pb.set_board_id(msgdds.board_id());
    }

void convert::protobuf::from_dds(
        const advrf_interfaces::msg::dds_::PdoAuxCmd_& msgdds,
        iit::advrf::PDOs_aux_cmd& pb)
    {
   for (const auto& aux_cmd : msgdds.aux_cmds()) {
       auto* pb_aux_cmd = pb.add_aux_cmds();
       pb_aux_cmd->set_type(
           static_cast<iit::advrf::PDOs_aux_cmd::Aux_cmd::Type>(aux_cmd.type()));
       pb_aux_cmd->set_board_id(aux_cmd.board_id());
   }
    }
void convert::protobuf::from_dds(
        const advrf_interfaces::msg::dds_::ReplCmd_Content_& request,
        iit::advrf::Repl_cmd& pb)
    {
        pb.set_type(static_cast<iit::advrf::CmdType>(request.type()));

        from_dds(request.trajectory_cmd(), *pb.mutable_trajectory_cmd());
        from_dds(request.ctrl_cmd(), *pb.mutable_ctrl_cmd());
        from_dds(request.flash_cmd(), *pb.mutable_flash_cmd());
        from_dds(request.ecat_master_cmd(), *pb.mutable_ecat_master_cmd());
        from_dds(request.foe_master(), *pb.mutable_foe_master());
        from_dds(request.trj_queue_cmd(), *pb.mutable_trj_queue_cmd());
        from_dds(request.slave_sdo_cmd(), *pb.mutable_slave_sdo_cmd());
        from_dds(request.slave_sdo_info(), *pb.mutable_slave_sdo_info());
        from_dds(request.motors_pdo_cmd(), *pb.mutable_motors_pdo_cmd());
        from_dds(request.slave_registry_write(), *pb.mutable_slave_registry_write());
        from_dds(request.pdos_aux_cmd(), *pb.mutable_pdos_aux_cmd());
    }

void convert::protobuf::from_dds(
        const advrf_interfaces::msg::dds_::ReplCmd_Content_Vector_& request,
        iit::advrf::Repl_cmd_vector& pb)
    {
        for (const auto& content : request.requests()) {
            auto* pb_content = pb.add_requests();
            from_dds(content, *pb_content);
        }
    }

void convert::protobuf::from_dds(
        const advrf_interfaces::srv::dds_::ReplCmd_Request_& request,
        iit::advrf::Repl_cmd& pb)
    {
    from_dds(request.request(), pb);
    }

void convert::dds::from_protobuf(uint64_t timestamp_ns, builtin_interfaces::msg::dds_::Time_& msgdds)
    {
   msgdds.sec() = static_cast<int32_t>(timestamp_ns / 1'000'000'000ULL);
   msgdds.nanosec() = static_cast<uint32_t>(timestamp_ns % 1'000'000'000ULL);
    }

void convert::dds::from_protobuf(const iit::advrf::Ec_slave_pdo& pb, std_msgs::msg::dds_::Header_& dds_time)
    {
   dds_time.frame_id() = pb.has_header() ? pb.header().str_id() : "";
   if (pb.has_header() && pb.header().has_stamp()) {
       const auto& s = pb.header().stamp();
       dds_time.stamp().sec() = s.sec();
       dds_time.stamp().nanosec() = s.nsec();
   } else {
       dds_time.stamp().sec() = 0;
       dds_time.stamp().nanosec() = 0;
   }
    }

void convert::dds::from_protobuf(const iit::advrf::Cmd_reply& pb, advrf_interfaces::srv::dds_::ReplCmd_Response_& dds_response)
    {
   dds_response.type() = static_cast<uint8_t>(pb.type());
   dds_response.msg() = pb.msg();
   dds_response.header().frame_id() = pb.header().str_id();
   dds_response.header().stamp().sec() = pb.header().stamp().sec();
   dds_response.header().stamp().nanosec() = pb.header().stamp().nsec();
   dds_response.pdo() = pb.pdo();
    }

void convert::dds::from_protobuf(const iit::advrf::ImuVN_rx_pdo& pb, advrf_interfaces::msg::dds_::Imu_& ddsmsg)
    {
   ddsmsg.linear_acceleration().x()  = pb.x_acc();
   ddsmsg.linear_acceleration().y()  = pb.y_acc();
   ddsmsg.linear_acceleration().z()  = pb.z_acc();
    
   ddsmsg.angular_velocity().x()     = pb.x_rate();
   ddsmsg.angular_velocity().y()     = pb.y_rate();
   ddsmsg.angular_velocity().z()     = pb.z_rate();
   
   ddsmsg.orientation().x()          = pb.x_quat();
   ddsmsg.orientation().y()          = pb.y_quat();
   ddsmsg.orientation().z()          = pb.z_quat();
   ddsmsg.orientation().w()          = pb.w_quat();
   
   ddsmsg.imu_ts()                   = pb.imu_ts();
   ddsmsg.temperature()              = pb.temperature();
   ddsmsg.digital_in()               = pb.digital_in();
   ddsmsg.fault()                    = pb.fault();
   ddsmsg.rtt()                      = pb.rtt();
    }

void convert::dds::from_protobuf(const iit::advrf::Cia402_rx_pdo& pb, sensor_msgs::msg::dds_::JointState_& ddsmsg)
    {
   ddsmsg.position().push_back(pb.link_pos());
   ddsmsg.velocity().push_back(pb.link_vel());
   ddsmsg.effort().push_back(pb.torque());
    }

void convert::dds::from_protobuf(const iit::advrf::Cia402_rx_pdo& pb, advrf_interfaces::msg::dds_::Motor_& ddsmsg)
    {
   // from protobuf to dds
   ddsmsg.statusword().push_back(pb.statusword());
   ddsmsg.modes_of_op().push_back(pb.modes_of_op());
   ddsmsg.motor_pos().push_back(pb.motor_pos());
   ddsmsg.motor_vel().push_back(pb.motor_vel());
   ddsmsg.link_pos().push_back(pb.link_pos());
   ddsmsg.link_vel().push_back(pb.link_vel());
   ddsmsg.current().push_back(pb.current());
   ddsmsg.torque().push_back(pb.torque());
   ddsmsg.demanded_pos().push_back(pb.demanded_pos());
   ddsmsg.demanded_vel().push_back(pb.demanded_vel());
   ddsmsg.demanded_torque().push_back(pb.demanded_torque());
   ddsmsg.demanded_current().push_back(pb.demanded_current());
   ddsmsg.control_effort().push_back(pb.control_effort());
   ddsmsg.motor_temp().push_back(pb.motor_temp());
   ddsmsg.drive_temp().push_back(pb.drive_temp());
   ddsmsg.error_code().push_back(pb.error_code());
   ddsmsg.error_report().push_back(pb.error_report());
   // default
   ddsmsg.fault().push_back(0);
   ddsmsg.rtt().push_back(0);

    }

void convert::dds::from_protobuf(const iit::advrf::Motor_xt_rx_pdo& pb, sensor_msgs::msg::dds_::JointState_& ddsmsg)
    {
   ddsmsg.position().push_back(pb.link_pos());
   ddsmsg.velocity().push_back(pb.link_vel());
   ddsmsg.effort().push_back(pb.torque());
    }

void convert::dds::from_protobuf(const iit::advrf::Motor_xt_rx_pdo& pb, advrf_interfaces::msg::dds_::Motor_& ddsmsg)
    {
   // from protobuf to dds
   ddsmsg.motor_pos().push_back(pb.motor_pos());
   ddsmsg.link_pos().push_back(pb.link_pos());
   ddsmsg.motor_vel().push_back(pb.motor_vel());
   ddsmsg.link_vel().push_back(pb.link_vel());
   ddsmsg.torque().push_back(pb.torque());
   ddsmsg.motor_temp().push_back(pb.motor_temp());
   ddsmsg.fault().push_back(pb.fault());
   ddsmsg.rtt().push_back(pb.rtt());
   // default
   ddsmsg.statusword().push_back(0);
   ddsmsg.modes_of_op().push_back(0);
   ddsmsg.current().push_back(0);
   ddsmsg.demanded_pos().push_back(0);
   ddsmsg.demanded_vel().push_back(0);
   ddsmsg.demanded_torque().push_back(0);
   ddsmsg.demanded_current().push_back(0);
   ddsmsg.control_effort().push_back(0);
   ddsmsg.drive_temp().push_back(0);
   ddsmsg.error_code().push_back(0);
   ddsmsg.error_report().push_back(0);
    }

void convert::dds::from_protobuf(const iit::advrf::Motor_rx_pdo& pb, sensor_msgs::msg::dds_::JointState_& ddsmsg)
    {
   ddsmsg.position().push_back(pb.link_pos());
   ddsmsg.velocity().push_back(0.0);
   ddsmsg.effort().push_back(static_cast<float>(pb.torque()));
    }

void convert::dds::from_protobuf(const iit::advrf::Motor_rx_pdo& pb, advrf_interfaces::msg::dds_::Motor_& ddsmsg)
    {
   // from protobuf to dds
   ddsmsg.motor_pos().push_back(pb.motor_pos());
   ddsmsg.link_pos().push_back(pb.link_pos());
   ddsmsg.torque().push_back(pb.torque());
   ddsmsg.fault().push_back(pb.fault());
   ddsmsg.rtt().push_back(pb.rtt());
   // default
   ddsmsg.motor_vel().push_back(0);
   ddsmsg.link_vel().push_back(0);
   ddsmsg.motor_temp().push_back(0);
   ddsmsg.statusword().push_back(0);
   ddsmsg.modes_of_op().push_back(0);
   ddsmsg.current().push_back(0);
   ddsmsg.demanded_pos().push_back(0);
   ddsmsg.demanded_vel().push_back(0);
   ddsmsg.demanded_torque().push_back(0);
   ddsmsg.demanded_current().push_back(0);
   ddsmsg.control_effort().push_back(0);
   ddsmsg.drive_temp().push_back(0);
   ddsmsg.error_code().push_back(0);
   ddsmsg.error_report().push_back(0);
    }

void convert::dds::from_protobuf(const iit::advrf::HyqKnee_rx_pdo& pb, sensor_msgs::msg::dds_::JointState_& ddsmsg)
    {
   ddsmsg.position().push_back(0.0);
   ddsmsg.velocity().push_back(0.0);
   ddsmsg.effort().push_back(pb.force());
    }

void convert::dds::from_protobuf(const iit::advrf::HyqKnee_rx_pdo& pb, advrf_interfaces::msg::dds_::Valve_& ddsmsg)
    {
   ddsmsg.force().push_back(pb.force());
   ddsmsg.pressure1().push_back(pb.pressure_1());
   ddsmsg.pressure2().push_back(pb.pressure_2());
   ddsmsg.encoder_position().push_back(pb.encoder_position());
   ddsmsg.current().push_back(pb.current());
   ddsmsg.temperature().push_back(pb.temperature());
   ddsmsg.fault().push_back(pb.fault());
   ddsmsg.rtt().push_back(pb.rtt());
   ddsmsg.op_idx_ack().push_back(pb.op_idx_ack());
   ddsmsg.aux().push_back(pb.aux());
   ddsmsg.current_ref_fb().push_back(pb.current_ref_fb());
   ddsmsg.position_ref_fb().push_back(pb.position_ref_fb());
   ddsmsg.force_ref_fb().push_back(pb.force_ref_fb());
    }

void convert::dds::from_protobuf(const iit::advrf::HyqHpu_rx_pdo& pb, advrf_interfaces::msg::dds_::Pump_& ddsmsg)
    {
   
   ddsmsg.motor_current()       = pb.motor_current();
   ddsmsg.motor_speed()         = pb.motor_speed();
   ddsmsg.pressure1()           = pb.pressure1();
   ddsmsg.pressure2()           = pb.pressure2();
   ddsmsg.temperature()         = pb.temperature();
   ddsmsg.mosfet_temperature()  = pb.mosfet_temperature();
   ddsmsg.motor_temperature()   = pb.motor_temperature();
   ddsmsg.fault()               = pb.fault();
   ddsmsg.rtt()                 = pb.rtt();
   ddsmsg.op_idx_ack()          = pb.op_idx_ack();
   ddsmsg.aux()                 = pb.aux();
    }

void convert::dds::from_protobuf(const iit::advrf::Gripper_rx_pdo& pb, advrf_interfaces::msg::dds_::Gripper_& ddsmsg)
    {
   ddsmsg.statusword().push_back(pb.statusword());
   ddsmsg.motor_pos().push_back(pb.motor_pos());
   ddsmsg.link_pos().push_back(pb.link_pos());
   ddsmsg.demanded_pos().push_back(pb.demanded_pos());
   ddsmsg.demanded_vel().push_back(pb.demanded_vel());
   ddsmsg.error_code().push_back(pb.error_code());
    }

void convert::dds::from_protobuf(const iit::advrf::Gripper_rx_pdo& pb, sensor_msgs::msg::dds_::JointState_ & ddsmsg)
    {
   ddsmsg.position().push_back(pb.link_pos());
   ddsmsg.velocity().push_back(0.0);
   ddsmsg.effort().push_back(0.0);
    }

void convert::dds::from_protobuf(const iit::advrf::PowF28M36_rx_pdo& pb, advrf_interfaces::msg::dds_::PowerBoard_ & ddsmsg)
    {
   ddsmsg.v_batt()         = pb.v_batt();
   ddsmsg.v_load()         = pb.v_load();
   ddsmsg.i_load()         = pb.i_load();
   ddsmsg.temperature()    = pb.temp_pcb();
   ddsmsg.temp_heatsink()  = pb.temp_heatsink();
   ddsmsg.temp_batt()      = pb.temp_batt();
   ddsmsg.status()         = pb.status();
   ddsmsg.fault()          = pb.fault();
   ddsmsg.rtt()            = static_cast<uint32_t>(pb.rtt());
   ddsmsg.op_idx_ack()     = pb.op_idx_ack();
   ddsmsg.aux()            = pb.aux();
    }

void convert::dds::from_protobuf(const iit::advrf::FT6_rx_pdo& pb, advrf_interfaces::msg::dds_::ForceTorque_& ddsmsg)
    {
   auto& wrench = ddsmsg.wrench();
   wrench.force().x() = pb.force_x();
   wrench.force().y() = pb.force_y();
   wrench.force().z() = pb.force_z();

   wrench.torque().x() = pb.torque_x();
   wrench.torque().y() = pb.torque_y();
   wrench.torque().z() = pb.torque_z();

   ddsmsg.fault()                 = pb.fault();
   ddsmsg.rtt()                   = pb.rtt();
   ddsmsg.op_idx_ack()            = pb.op_idx_ack();
   ddsmsg.aux()                   = pb.aux();
    }

