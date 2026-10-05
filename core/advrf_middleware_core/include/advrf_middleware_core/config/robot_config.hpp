#pragma once

#include <cstdint>
#include <string>
#include <optional>
#include <yaml-cpp/yaml.h>
#include <iostream>

#include <advrf_middleware_core/utils/channel.hpp>
#include <advrf_middleware_core/utils/pdo_utils.hpp>

namespace advrf::middleware::config
{

/**
 * @brief Runtime configuration of a robot middleware instance.
 *
 * Values are loaded from the EtherCAT configuration and joint-ID map YAML
 * files.
 */
struct RobotConfig {
    /// Robot name read from ecat config file.
    std::string robot_name {"NoNe"};
    /// DDS domain ID.
    uint32_t domain_id {0};
    /// Middleware namespace used for topic construction.
    std::string ns {""};
    /// Whether middleware data should be declared to ROS2.
    bool declare_to_ros {false};
    /// Mapping from EtherCAT IDs to configured joint/device names.
    std::unordered_map<advrf::middleware::pdo::EcatId, std::string> map_ecat_id;
};

/**
 * @brief Load general robot and DDS settings from an EtherCAT YAML file.
 *
 * Missing YAML keys preserve the current values in @p cfg.
 * YAML parsing failures are reported to stderr and leave already-loaded
 * configuration values unchanged.
 */
inline void load_from_ecat_config(RobotConfig& cfg, const std::string& ecat_config_path) {
    try {
        YAML::Node root = YAML::LoadFile(ecat_config_path);
        YAML::Node ecat_board_ctrl = root["ec_board_ctrl"];
        if (ecat_board_ctrl) {
            cfg.robot_name = ecat_board_ctrl["robot_name"] ? ecat_board_ctrl["robot_name"].as<std::string>() : cfg.robot_name;
        }
        YAML::Node dds = root["dds"];
        if (dds) {
            cfg.ns = dds["namespace"] ? dds["namespace"].as<std::string>() : cfg.ns;
            cfg.domain_id = dds["domain"] ? dds["domain"].as<uint32_t>() : cfg.domain_id;
            cfg.declare_to_ros = dds["declare_to_ros"] ? dds["declare_to_ros"].as<bool>() : cfg.declare_to_ros;
            
            YAML::Node qos = dds["qos"];
            if(qos) {
                // TODO
            }
        
        }
    } catch (const YAML::Exception& e) {
        std::cerr << "[RobotConfig] Failed to parse '" << ecat_config_path << "': " << e.what() << '\n';
    }
}

/**
 * @brief Load EtherCAT-ID-to-device-name mappings from a YAML file.
 *
 * Expects a `joint_map` YAML entry.
 */
inline void load_from_robot_id_map(RobotConfig& cfg, const std::string& ecat_robot_id_map_path) {
    try {
        YAML::Node root = YAML::LoadFile(ecat_robot_id_map_path);
        const YAML::Node joints_array = root["joint_map"];
        if (joints_array) {
            for (const auto& j : joints_array) {
                cfg.map_ecat_id[j.first.as<int>()] = j.second.as<std::string>();
            }
        }
    } catch (const YAML::Exception& e) {
        std::cerr << "[RobotConfig] Failed to parse '" << ecat_robot_id_map_path << "': " << e.what() << '\n';
    }
}

/**
 * @brief Load a complete robot configuration from the two YAML files.
 */
inline std::optional<RobotConfig> load_robot_config(const std::string& ecat_map_id_path, 
                                                    const std::string& ecat_config_path) {
    RobotConfig cfg;
    load_from_robot_id_map(cfg, ecat_map_id_path);
    load_from_ecat_config(cfg, ecat_config_path);
    return cfg;
}

/**
 * @brief Return all EtherCAT PDO IDs configured for the robot.
 */
inline std::set<uint32_t> extract_pdo_ids(const RobotConfig& cfg) {
    std::set<uint32_t> pdo_ids;
    const auto add_ids = [&pdo_ids](const auto& devices) {
        for (const auto& [id, name] : devices) {
            pdo_ids.insert(id);
        }
    };
    add_ids(cfg.map_ecat_id);
    return pdo_ids;
}

/**
 * @brief Fluent builder for composing a @ref RobotConfig from YAML sources.
 */
class RobotConfigBuilder {

public:
    /// Merge general robot and DDS settings from an EtherCAT configuration.
    RobotConfigBuilder& from_ecat_config(const std::string& ecat_config_path){
        load_from_ecat_config(robot_config_, ecat_config_path);
        return *this;
    }

    /// Merge EtherCAT-ID-to-device-name mappings from a joint-map file.
    RobotConfigBuilder& from_robot_id_map(const std::string& ecat_robot_id_map_path){
        load_from_robot_id_map(robot_config_, ecat_robot_id_map_path);
        return *this;
    }

    /// Return the accumulated configuration.
    const RobotConfig& build() {
        return robot_config_;
    }

private:
    RobotConfig robot_config_;
};

}