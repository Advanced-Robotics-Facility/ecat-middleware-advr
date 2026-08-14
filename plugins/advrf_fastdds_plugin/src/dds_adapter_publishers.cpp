#include "advrf_fastdds_plugin/adapters/dds_adapter_publishers.hpp"
#include "advrf_fastdds_plugin/adapters/dds_pdo_publisher.hpp"

#include <vector>
#include <string>
#include <unordered_map>
#include <cstdint>

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

std::unordered_map<uint32_t, std::string> build_name_map(const RobotConfig& cfg) {
    std::unordered_map<uint32_t, std::string> out;
    auto add = [&](const std::vector<JointConfig>& v) {
        for (const auto& j : v)
            if (j.ecat_id >= 0) 
                out[static_cast<uint32_t>(j.ecat_id)] = j.name;
    };

    add(cfg.motors); 
    add(cfg.grippers); 
    add(cfg.valves);
    add(cfg.imus); 
    add(cfg.power_boards); 
    add(cfg.pumps); 
    add(cfg.force_torques);

    return out;
}


void DDSAdapterPublishers::init_ros_graph_bridge(
    const RobotConfig& robot_config,
    eprosima::fastdds::dds::DomainParticipant* dp)
{
    if (!robot_config.declare_to_ros) {
        return;
    }

    const std::string node_namespace = FastRosGraphBridge::build_node_namespace(
        robot_config.ns,
        robot_config.robot_name
    );

    ros_graph_bridge_ = std::make_unique<FastRosGraphBridge>(
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
    eprosima::fastdds::dds::DomainParticipant* dp) 
{
    const auto id_to_name = build_name_map(robot_config);

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

    for(const auto& id_name_pair : id_to_name) {
        LOG_INFO("Mapping ecat_id {} to name {}", id_name_pair.first, id_name_pair.second);
    }

   
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
