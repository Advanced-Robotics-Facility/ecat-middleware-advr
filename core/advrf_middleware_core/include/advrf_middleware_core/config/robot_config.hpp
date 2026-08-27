#pragma once

#include <cstdint>
#include <string>
#include <optional>
#include <yaml-cpp/yaml.h>
#include <iostream>

#include <advrf_middleware_core/utils/channel.hpp>

using EcatId = std::uint32_t;

struct RobotConfig {
    std::string robot_name {"NoNe"};
    uint32_t domain_id {0};
    std::string ns {""};
    bool declare_to_ros {false};
    std::unordered_map<EcatId, std::string> map_ecat_id;
};


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
        }
    } catch (const YAML::Exception& e) {
        std::cerr << "[RobotConfig] Failed to parse '" << ecat_config_path << "': " << e.what() << '\n';
    }
}

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


inline std::optional<RobotConfig> load_robot_config(const std::string& ecat_map_id_path, 
                                                    const std::string& ecat_config_path) {
    RobotConfig cfg;
    load_from_robot_id_map(cfg, ecat_map_id_path);
    load_from_ecat_config(cfg, ecat_config_path);
    return cfg;
}


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