#include "advrf_cyclonedds_plugin/adapters/dds_adapter_publishers.hpp"
#include "advrf_cyclonedds_plugin/adapters/dds_adapter_bridges.hpp"

#include <vector>

std::vector<int> extract_ids(const std::vector<JointConfig>& joints) {
    std::vector<int> ids;
    ids.reserve(joints.size());
    for (const auto& j : joints)
        ids.push_back(j.ecat_id);
    return ids;
}

std::vector<DDSAdapterPublishers::EcatId> to_ecat_id(const std::vector<int>& ids) {
    std::vector<DDSAdapterPublishers::EcatId> out;
    out.reserve(ids.size());
    for (int id : ids) {
        if (id < 0) {
            LOG_ERROR("Negative ecat_id {} in config, skipping", id);
            continue;
        }
        out.push_back(static_cast<DDSAdapterPublishers::EcatId>(id));
    }
    return out;
}

bool DDSAdapterPublishers::init(
    const config::ConfigTopics& config_topics, 
    const RobotConfig& robot_config,
    dds::domain::DomainParticipant& dp) 
{
    const auto motor_ids = to_ecat_id(extract_ids(robot_config.motors));
    const auto valve_ids = to_ecat_id(extract_ids(robot_config.valves));
    const auto gripper_ids = to_ecat_id(extract_ids(robot_config.grippers));
    const auto imu_ids = to_ecat_id(extract_ids(robot_config.imus));
    const auto power_board_ids = to_ecat_id(extract_ids(robot_config.power_boards));
    const auto pump_ids = to_ecat_id(extract_ids(robot_config.pumps));
    const auto force_torque_ids = to_ecat_id(extract_ids(robot_config.force_torques));

    std::vector<EcatId> joint_ids = motor_ids;
    joint_ids.insert(joint_ids.end(), gripper_ids.begin(), gripper_ids.end());
    joint_ids.insert(joint_ids.end(), valve_ids.begin(), valve_ids.end());

    if (joint_ids.size() != 0) {
        register_publisher<JointStatePublisher>(
                {Channel::Motor, Channel::Gripper, Channel::Valve}, joint_ids)
            .init(config_topics.state.jointState(), dp);
    }

    if (motor_ids.size() != 0) {
        register_publisher<MotorsPublisher>(
            {Channel::Motor}, motor_ids)
        .init(config_topics.state.motor(), dp);
    }

    if (valve_ids.size() != 0) {
        register_publisher<ValvePublisher>(
                {Channel::Valve}, valve_ids)
            .init(config_topics.state.valve(), dp);
    }

    if (gripper_ids.size() != 0) {
        register_publisher<GripperPublisher>(
                {Channel::Gripper}, gripper_ids)
            .init(config_topics.state.gripper(), dp);
    }

    if (imu_ids.size() != 0) {
        register_publisher<ImuPublisher>(
                {Channel::Imu})
            .init(config_topics.state.imu(), dp);
    }

    if (power_board_ids.size() != 0) {
        register_publisher<PowerBoardPublisher>(
                {Channel::PowerBoard})
            .init(config_topics.state.powerBoard(), dp);
    }

    if (pump_ids.size() != 0) {
        register_publisher<PumpPublisher>(
                {Channel::Pump})
            .init(config_topics.state.pump(), dp);
    }
    
    if (force_torque_ids.size() != 0) {
        register_publisher<ForceTorquePublisher>(
                {Channel::ForceTorque})
            .init(config_topics.state.forceTorque(), dp);
    }
    
    return true;
}
