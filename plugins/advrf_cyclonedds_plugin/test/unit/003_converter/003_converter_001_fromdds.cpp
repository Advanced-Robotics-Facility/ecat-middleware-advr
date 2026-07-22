#include <cassert>
#include <iostream>

#include "advrf_cyclonedds_plugin/converter.hpp"

#include <std_msgs/msg/Header.hpp>
#include <advrf_interfaces/msg/CtrlCmd.hpp>
#include <advrf_interfaces/msg/Gains.hpp>

constexpr double EPS = 1e-6;

template<typename T, typename U>
inline void assert_float_eq(T actual, U expected)
{
    assert(std::abs(static_cast<double>(actual) -
                    static_cast<double>(expected)) < EPS);
}


int main()
{
    advrf_interfaces::srv::dds_::ReplCmd_Request_ request;
    request.type() = advrf_interfaces::srv::dds_::CTRL_CMD;

    // Header
    request.header().frame_id() = "base_link";
    request.header().stamp().sec() = 123;
    request.header().stamp().nanosec() = 456;

    // TrajectoryCmd
    request.trajectory_cmd().type() = advrf_interfaces::msg::dds_::SMOOTHER;
    request.trajectory_cmd().name() = "traj_test";
    request.trajectory_cmd().board_id() = 7;

    request.trajectory_cmd().smooth_par().x().push_back(1.0);
    request.trajectory_cmd().smooth_par().x().push_back(2.0);
    request.trajectory_cmd().smooth_par().y().push_back(3.0);
    request.trajectory_cmd().smooth_par().y().push_back(4.0);

    request.trajectory_cmd().period_par().freq() = 100.0;
    request.trajectory_cmd().period_par().ampl() = 5.0;
    request.trajectory_cmd().period_par().teta() = 6.0;
    request.trajectory_cmd().period_par().secs() = 7.0;

    request.trajectory_cmd().homing_par().x().push_back(8.0);

    request.trajectory_cmd().smooth_vel_par().dt() = 0.01;
    request.trajectory_cmd().smooth_vel_par().p0() = 1.0;
    request.trajectory_cmd().smooth_vel_par().v0() = 2.0;
    request.trajectory_cmd().smooth_vel_par().pt() = 3.0;
    request.trajectory_cmd().smooth_vel_par().vt() = 4.0;
    request.trajectory_cmd().smooth_vel_par().ds() = 5.0;
    request.trajectory_cmd().smooth_vel_par().magic() = 6.0;

    // CtrlCmd
    request.ctrl_cmd().type() = advrf_interfaces::msg::dds_::CTRL_SET_POSITION;
    request.ctrl_cmd().board_id() = 12;
    request.ctrl_cmd().value() = 42.5f;

    request.ctrl_cmd().gains().type() = 1;
    request.ctrl_cmd().gains().pos_kp() = 10.0;
    request.ctrl_cmd().gains().pos_kd() = 20.0;
    request.ctrl_cmd().gains().tor_kp() = 30.0;
    request.ctrl_cmd().gains().tor_ki() = 40.0;
    request.ctrl_cmd().gains().tor_kd() = 50.0;

    // FlashCmd
    request.flash_cmd().type() = 1;
    request.flash_cmd().board_id() = 2;

    // EcatMasterCmd
    request.ecat_master_cmd().type() = advrf_interfaces::msg::dds_::START_MASTER;

    auto &arg = request.ecat_master_cmd().args().emplace_back();
    arg.name() = "iface";
    arg.value() = "eth0";

    // FOE Master
    request.foe_master().filename() = "firmware.bin";
    request.foe_master().password() = 123456;
    request.foe_master().mcu_type() = "STM32";
    request.foe_master().slave_pos() = 3;
    request.foe_master().board_id() = 4;

    // TrjQueueCmd
    request.trj_queue_cmd().type() = advrf_interfaces::msg::dds_::PUSH_QUE;
    request.trj_queue_cmd().trj_names().push_back("traj1");
    request.trj_queue_cmd().trj_names().push_back("traj2");

    // SlaveSdoCmd
    request.slave_sdo_cmd().board_id() = 6;
    request.slave_sdo_cmd().rd_sdo().push_back("StatusWord");

    auto &wr = request.slave_sdo_cmd().wr_sdo().emplace_back();
    wr.name() = "ControlWord";
    wr.value() = "0x000F";

    // SlaveSdoInfo
    request.slave_sdo_info().type() = advrf_interfaces::msg::dds_::SDO_NAME;
    request.slave_sdo_info().board_id() = 5;

    // MotorsPdoCmd
    auto &motor = request.motors_pdo_cmd().motors_pdo().emplace_back();

    motor.motor_id() = 1;
    motor.pos_ref() = 1.5f;
    motor.vel_ref() = 2.5f;
    motor.tor_ref() = 3.5f;

    motor.gains().type() = 2;
    motor.gains().pos_kp() = 100;
    motor.gains().pos_kd() = 200;
    motor.gains().tor_kp() = 300;
    motor.gains().tor_ki() = 400;
    motor.gains().tor_kd() = 500;

    // SlaveRegistryWrite
    request.slave_registry_write().type() = 1;
    request.slave_registry_write().board_id() = 10;

    // PdoAuxCmd
    auto &aux = request.pdos_aux_cmd().aux_cmds().emplace_back();
    aux.type() = advrf_interfaces::msg::dds_::LED_ON;
    aux.board_id() = 99;

    iit::advrf::Repl_cmd pb;
    convert::protobuf::from_dds(request, pb);

    assert(pb.type() == static_cast<iit::advrf::CmdType>(request.type()));

    //
    // TrajectoryCmd
    //
    assert(pb.trajectory_cmd().type() ==
        static_cast<iit::advrf::Trajectory_cmd::Type>(request.trajectory_cmd().type()));

    assert(pb.trajectory_cmd().name() == "traj_test");
    assert(pb.trajectory_cmd().board_id() == 7);

    assert(pb.trajectory_cmd().smooth_par().x_size() == 2);
    assert_float_eq(pb.trajectory_cmd().smooth_par().x(0), 1.0);
    assert_float_eq(pb.trajectory_cmd().smooth_par().x(1), 2.0);

    assert(pb.trajectory_cmd().smooth_par().y_size() == 2);
    assert_float_eq(pb.trajectory_cmd().smooth_par().y(0), 3.0);
    assert_float_eq(pb.trajectory_cmd().smooth_par().y(1), 4.0);

    assert_float_eq(pb.trajectory_cmd().period_par().freq(), 100.0);
    assert_float_eq(pb.trajectory_cmd().period_par().ampl(), 5.0);
    assert_float_eq(pb.trajectory_cmd().period_par().teta(), 6.0);
    assert_float_eq(pb.trajectory_cmd().period_par().secs(), 7.0);

    assert(pb.trajectory_cmd().homing_par().x_size() == 1);
    assert_float_eq(pb.trajectory_cmd().homing_par().x(0), 8.0);

    assert_float_eq(pb.trajectory_cmd().smooth_vel_par().dt(), 0.01);
    assert_float_eq(pb.trajectory_cmd().smooth_vel_par().p0(), 1.0);
    assert_float_eq(pb.trajectory_cmd().smooth_vel_par().v0(), 2.0);
    assert_float_eq(pb.trajectory_cmd().smooth_vel_par().pt(), 3.0);
    assert_float_eq(pb.trajectory_cmd().smooth_vel_par().vt(), 4.0);
    assert_float_eq(pb.trajectory_cmd().smooth_vel_par().ds(), 5.0);
    assert_float_eq(pb.trajectory_cmd().smooth_vel_par().magic(), 6.0);

    //
    // CtrlCmd
    //
    assert(pb.ctrl_cmd().type() ==
        static_cast<iit::advrf::Ctrl_cmd::Type>(request.ctrl_cmd().type()));

    assert(pb.ctrl_cmd().board_id() == 12);
    assert_float_eq(pb.ctrl_cmd().value(), 42.5);

    assert(pb.ctrl_cmd().gains().type() ==
        static_cast<iit::advrf::Gains::Type>(1));

    assert_float_eq(pb.ctrl_cmd().gains().pos_kp(), 10.0);
    assert_float_eq(pb.ctrl_cmd().gains().pos_kd(), 20.0);
    assert_float_eq(pb.ctrl_cmd().gains().tor_kp(), 30.0);
    assert_float_eq(pb.ctrl_cmd().gains().tor_ki(), 40.0);
    assert_float_eq(pb.ctrl_cmd().gains().tor_kd(), 50.0);

    //
    // FlashCmd
    //
    assert(pb.flash_cmd().type() ==
        static_cast<iit::advrf::Flash_cmd::Type>(1));

    assert(pb.flash_cmd().board_id() == 2);

    //
    // EcatMasterCmd
    //
    assert(pb.ecat_master_cmd().type() ==
        static_cast<iit::advrf::Ecat_Master_cmd::Type>(
            request.ecat_master_cmd().type()));

    assert(pb.ecat_master_cmd().args_size() == 1);
    assert(pb.ecat_master_cmd().args(0).name() == "iface");
    assert(pb.ecat_master_cmd().args(0).value() == "eth0");

    //
    // FOE Master
    //
    assert(pb.foe_master().filename() == "firmware.bin");
    assert(pb.foe_master().password() == 123456);
    assert(pb.foe_master().mcu_type() == "STM32");
    assert(pb.foe_master().slave_pos() == 3);
    assert(pb.foe_master().board_id() == 4);

    //
    // TrjQueueCmd
    //
    assert(pb.trj_queue_cmd().type() ==
        static_cast<iit::advrf::Trj_queue_cmd::Type>(
            request.trj_queue_cmd().type()));

    assert(pb.trj_queue_cmd().trj_names_size() == 2);
    assert(pb.trj_queue_cmd().trj_names(0) == "traj1");
    assert(pb.trj_queue_cmd().trj_names(1) == "traj2");

    //
    // SlaveSdoCmd
    //
    assert(pb.slave_sdo_cmd().board_id() == 6);

    assert(pb.slave_sdo_cmd().rd_sdo_size() == 1);
    assert(pb.slave_sdo_cmd().rd_sdo(0) == "StatusWord");

    assert(pb.slave_sdo_cmd().wr_sdo_size() == 1);
    assert(pb.slave_sdo_cmd().wr_sdo(0).name() == "ControlWord");
    assert(pb.slave_sdo_cmd().wr_sdo(0).value() == "0x000F");

    //
    // SlaveSdoInfo
    //
    assert(pb.slave_sdo_info().type() ==
        static_cast<iit::advrf::Slave_SDO_info::Type>(
            request.slave_sdo_info().type()));

    assert(pb.slave_sdo_info().board_id() == 5);

    //
    // MotorsPdoCmd
    //
    assert(pb.motors_pdo_cmd().motors_pdo_size() == 1);

    const auto& pb_motor = pb.motors_pdo_cmd().motors_pdo(0);

    assert(pb_motor.motor_id() == 1);

    assert_float_eq(pb_motor.pos_ref(), 1.5);
    assert_float_eq(pb_motor.vel_ref(), 2.5);
    assert_float_eq(pb_motor.tor_ref(), 3.5);

    assert(pb_motor.gains().type() ==
        static_cast<iit::advrf::Gains::Type>(2));

    assert_float_eq(pb_motor.gains().pos_kp(), 100);
    assert_float_eq(pb_motor.gains().pos_kd(), 200);
    assert_float_eq(pb_motor.gains().tor_kp(), 300);
    assert_float_eq(pb_motor.gains().tor_ki(), 400);
    assert_float_eq(pb_motor.gains().tor_kd(), 500);

    //
    // SlaveRegistryWrite
    //
    assert(pb.slave_registry_write().type() ==
        static_cast<iit::advrf::Slave_registry_write::Type>(1));

    assert(pb.slave_registry_write().board_id() == 10);

    //
    // PdoAuxCmd
    //
    assert(pb.pdos_aux_cmd().aux_cmds_size() == 1);

    assert(pb.pdos_aux_cmd().aux_cmds(0).type() ==
        static_cast<iit::advrf::PDOs_aux_cmd::Aux_cmd::Type>(
            request.pdos_aux_cmd().aux_cmds()[0].type()));

    assert(pb.pdos_aux_cmd().aux_cmds(0).board_id() == 99);
    
    std::cout << "\nAll converter tests passed.\n";
    return 0;
}