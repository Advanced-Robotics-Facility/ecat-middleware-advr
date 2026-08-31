#include "advrf_cyclonedds_plugin/adapters/dds_adapter_publishers.hpp"
#include "advrf_cyclonedds_plugin/adapters/dds_pdo_publisher.hpp"

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace advrf::cyclonedds_plugin {

void DDSAdapterPublishers::init_ros_graph_bridge(
    const config::RobotConfig& robot_config,
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
    const config::RobotConfig& robot_config,
    const EcatDiscover::EcatMap& ecat_map,
    dds::domain::DomainParticipant& dp)
{

    const auto filter = [&](ChannelRx channel) {
        std::vector<pdo_utils::EcatId> ids;
        ids.reserve(ecat_map.size());
        for (const auto& [id, device] : ecat_map) {
            if (ecat_map.find(id) == ecat_map.end() 
                || ecat_map.at(id).channel != channel) {
                continue;
            }
            ids.push_back(static_cast<pdo_utils::EcatId>(device.ecat_id));
        }
        return ids;
    };

    const auto motor_ids = filter(ChannelRx::Motor);
    const auto gripper_ids = filter(ChannelRx::Gripper);
    const auto valve_ids = filter(ChannelRx::Valve);
    
    std::vector<pdo_utils::EcatId> joint_ids;
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
            robot_config.map_ecat_id,
            config_topics.rx.jointState(),
            dp
        );
    }

    if (!motor_ids.empty()) {
        create_publisher<MotorsPublisher>(
            {ChannelRx::Motor},
            motor_ids,
            robot_config.map_ecat_id,
            config_topics.rx.motor(),
            dp
        );
    }

    if (!gripper_ids.empty()) {
        create_publisher<GripperPublisher>(
            {ChannelRx::Gripper},
            gripper_ids,
            robot_config.map_ecat_id,
            config_topics.rx.gripper(),
            dp
        );
    }

    if (!valve_ids.empty()) {
        create_publisher<ValvePublisher>(
            {ChannelRx::Valve},
            valve_ids,
            robot_config.map_ecat_id,
            config_topics.rx.valve(),
            dp
        );
    }

    // ---------------------------------------------------------------------
    // IMUs
    // ---------------------------------------------------------------------
    const auto imu_ids = filter(ChannelRx::Imu);
    if (imu_ids.size() == 1) {
        if(robot_config.map_ecat_id.find(imu_ids[0]) == robot_config.map_ecat_id.end()) {
            LOG_ERROR("IMU ECAT ID {} not found in robot configuration.", imu_ids[0]);
            return false;
        }
        const std::string& imu_name = robot_config.map_ecat_id.at(imu_ids[0]);
        create_publisher<ImuPublisher>(
            {ChannelRx::Imu},
            imu_ids,
            robot_config.map_ecat_id,
            config_topics.rx.imu(imu_name),
            dp
        );
    }else if (imu_ids.size() > 1) {
        LOG_ERROR("Multiple IMUs detected, but only one is supported. Found {} IMUs.", imu_ids.size());
        return false;
    }

    // ---------------------------------------------------------------------
    // Power boards
    // ---------------------------------------------------------------------

    const auto power_board_ids = filter(ChannelRx::PowerBoard);
    if (power_board_ids.size() == 1) {
        if(robot_config.map_ecat_id.find(power_board_ids[0]) == robot_config.map_ecat_id.end()) {
            LOG_ERROR("Power board ECAT ID {} not found in robot configuration.", power_board_ids[0]);
            return false;
        }
        const std::string& power_board_name = robot_config.map_ecat_id.at(power_board_ids[0]);
        create_publisher<PowerBoardPublisher>(
            {ChannelRx::PowerBoard},
            power_board_ids,
            robot_config.map_ecat_id,
            config_topics.rx.powerBoard(power_board_name),
            dp
        );
    } else if (power_board_ids.size() > 1) {
        LOG_ERROR("Multiple power boards detected, but only one is supported. Found {} power boards.", power_board_ids.size());
        return false;
    }

    // ---------------------------------------------------------------------
    // Pumps
    // ---------------------------------------------------------------------

    const auto pump_ids = filter(ChannelRx::Pump);
    if (pump_ids.size() == 1) {
        if(robot_config.map_ecat_id.find(pump_ids[0]) == robot_config.map_ecat_id.end()) {
            LOG_ERROR("Pump ECAT ID {} not found in robot configuration.", pump_ids[0]);
            return false;
        }
        const std::string& pump_name = robot_config.map_ecat_id.at(pump_ids[0]);
        create_publisher<PumpPublisher>(
            {ChannelRx::Pump},          
            pump_ids,
            robot_config.map_ecat_id,
            config_topics.rx.pump(pump_name),
            dp
        );
    } else if (pump_ids.size() > 1) {
        LOG_ERROR("Multiple pumps detected, but only one is supported. Found {} pumps.", pump_ids.size());
        return false;
    }

    // ---------------------------------------------------------------------
    // Force / Torque
    // ---------------------------------------------------------------------

    const auto force_torque_ids = filter(ChannelRx::ForceTorque);
    if (force_torque_ids.size() == 1) {
        if(robot_config.map_ecat_id.find(force_torque_ids[0]) == robot_config.map_ecat_id.end()) {
            LOG_ERROR("Force/Torque ECAT ID {} not found in robot configuration.", force_torque_ids[0]);
            return false;
        }
        const std::string& force_torque_name = robot_config.map_ecat_id.at(force_torque_ids[0]);
        create_publisher<ForceTorquePublisher>(
            {ChannelRx::ForceTorque},
            force_torque_ids,
            robot_config.map_ecat_id,
            config_topics.rx.forceTorque(force_torque_name),
            dp
        );
    } else if (force_torque_ids.size() > 1) {
        LOG_ERROR("Multiple force/torque sensors detected, but only one is supported. Found {} force/torque sensors.", force_torque_ids.size());
        return false;
    }


    init_ros_graph_bridge(robot_config, dp);

    return true;
}

}