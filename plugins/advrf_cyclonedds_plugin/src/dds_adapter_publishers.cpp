#include "advrf_cyclonedds_plugin/adapters/dds_adapter_publishers.hpp"
#include "advrf_cyclonedds_plugin/adapters/dds_pdo_publisher.hpp"

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace {

std::vector<DDSAdapterPublishers::EcatId> extract_ecat_ids(
    const std::vector<JointConfig>& joints)
{
    std::vector<DDSAdapterPublishers::EcatId> ids;
    ids.reserve(joints.size());

    for (const auto& joint : joints) {
        if (joint.ecat_id < 0) {
            LOG_ERROR(
                "Negative ecat_id {} for {}, skipping",
                joint.ecat_id,
                joint.name
            );
            continue;
        }

        ids.push_back(
            static_cast<DDSAdapterPublishers::EcatId>(joint.ecat_id)
        );
    }

    return ids;
}

std::unordered_map<uint32_t, std::string> build_name_map(
    const RobotConfig& config)
{
    std::unordered_map<uint32_t, std::string> names;

    const auto add = [&names](const std::vector<JointConfig>& joints) {
        for (const auto& joint : joints) {
            if (joint.ecat_id < 0) {
                continue;
            }

            names[static_cast<uint32_t>(joint.ecat_id)] = joint.name;
        }
    };

    add(config.motors);
    add(config.grippers);
    add(config.valves);
    add(config.imus);
    add(config.power_boards);
    add(config.pumps);
    add(config.force_torques);

    return names;
}

} // namespace

void DDSAdapterPublishers::init_ros_graph_bridge(
    const RobotConfig& robot_config,
    dds::domain::DomainParticipant& dp)
{
    if (!robot_config.declare_to_ros) {
        return;
    }

    const auto node_namespace = CycloneDDSRosGraphBridge::build_node_namespace(
        robot_config.ns,
        robot_config.robot_name
    );

    ros_graph_bridge_ = std::make_unique<CycloneDDSRosGraphBridge>(
        dp,
        "rx_node",
        node_namespace
    );

    for (auto& connectable : ros_connectables_) {
        connectable.get().connect_ros_graph_bridge(*ros_graph_bridge_);
    }
}

bool DDSAdapterPublishers::init(
    const config::ConfigTopics& config_topics,
    const RobotConfig& robot_config,
    dds::domain::DomainParticipant& dp)
{
    const auto id_to_name = build_name_map(robot_config);

    const auto motor_ids = extract_ecat_ids(robot_config.motors);
    const auto gripper_ids = extract_ecat_ids(robot_config.grippers);
    const auto valve_ids = extract_ecat_ids(robot_config.valves);

    std::vector<EcatId> joint_ids;
    joint_ids.reserve(
        motor_ids.size() +
        gripper_ids.size() +
        valve_ids.size()
    );

    joint_ids.insert(
        joint_ids.end(),
        motor_ids.begin(),
        motor_ids.end()
    );

    joint_ids.insert(
        joint_ids.end(),
        gripper_ids.begin(),
        gripper_ids.end()
    );

    joint_ids.insert(
        joint_ids.end(),
        valve_ids.begin(),
        valve_ids.end()
    );

    // ---------------------------------------------------------------------
    // Joint publishers
    // ---------------------------------------------------------------------

    if (!joint_ids.empty()) {
        create_publisher<JointStatePublisher>(
            {
                ChannelRx::Motor,
                ChannelRx::Gripper,
                ChannelRx::Valve
            },
            joint_ids,
            id_to_name,
            config_topics.rx.jointState(),
            dp
        );
    }

    if (!motor_ids.empty()) {
        create_publisher<MotorsPublisher>(
            {ChannelRx::Motor},
            motor_ids,
            id_to_name,
            config_topics.rx.motor(),
            dp
        );
    }

    if (!gripper_ids.empty()) {
        create_publisher<GripperPublisher>(
            {ChannelRx::Gripper},
            gripper_ids,
            id_to_name,
            config_topics.rx.gripper(),
            dp
        );
    }

    if (!valve_ids.empty()) {
        create_publisher<ValvePublisher>(
            {ChannelRx::Valve},
            valve_ids,
            id_to_name,
            config_topics.rx.valve(),
            dp
        );
    }

    // ---------------------------------------------------------------------
    // IMUs
    // ---------------------------------------------------------------------

    for (const auto& imu : robot_config.imus) {
        if (imu.ecat_id < 0) {
            continue;
        }

        create_publisher<ImuPublisher>(
            {ChannelRx::Imu},
            {static_cast<EcatId>(imu.ecat_id)},
            id_to_name,
            config_topics.rx.imu(imu.name),
            dp
        );
    }

    // ---------------------------------------------------------------------
    // Power boards
    // ---------------------------------------------------------------------

    for (const auto& power_board : robot_config.power_boards) {
        if (power_board.ecat_id < 0) {
            continue;
        }

        create_publisher<PowerBoardPublisher>(
            {ChannelRx::PowerBoard},
            {static_cast<EcatId>(power_board.ecat_id)},
            id_to_name,
            config_topics.rx.powerBoard(power_board.name),
            dp
        );
    }

    // ---------------------------------------------------------------------
    // Pumps
    // ---------------------------------------------------------------------

    for (const auto& pump : robot_config.pumps) {
        if (pump.ecat_id < 0) {
            continue;
        }

        create_publisher<PumpPublisher>(
            {ChannelRx::Pump},
            {static_cast<EcatId>(pump.ecat_id)},
            id_to_name,
            config_topics.rx.pump(pump.name),
            dp
        );
    }

    // ---------------------------------------------------------------------
    // Force / Torque
    // ---------------------------------------------------------------------

    for (const auto& force_torque : robot_config.force_torques) {
        if (force_torque.ecat_id < 0) {
            continue;
        }

        create_publisher<ForceTorquePublisher>(
            {ChannelRx::ForceTorque},
            {static_cast<EcatId>(force_torque.ecat_id)},
            id_to_name,
            config_topics.rx.forceTorque(force_torque.name),
            dp
        );
    }

    init_ros_graph_bridge(robot_config, dp);

    return true;
}