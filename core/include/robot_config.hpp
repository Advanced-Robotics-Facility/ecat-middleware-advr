#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <optional>
#include <yaml-cpp/yaml.h>
#include <iostream>

struct JointConfig {
    std::string name;
    int ecat_id {0};
};

struct RobotConfig {
    std::string robot_name;
    uint32_t domain_id {0};

    std::vector<JointConfig> joints;   
    std::vector<JointConfig> valves;   
    std::vector<JointConfig> motors;   
    std::vector<std::string> has_imu;  
    std::vector<std::string> has_power_board;  
    std::vector<std::string> has_force_torque; 
    std::vector<std::string> has_pump;  

    std::vector<std::string> joint_names() const {
        std::vector<std::string> out;
        out.reserve(joints.size());
        for (const auto& j : joints) out.push_back(j.name);
        return out;
    }

    std::vector<std::string> valve_names() const {
        std::vector<std::string> out;
        out.reserve(valves.size());
        for (const auto& v : valves) out.push_back(v.name);
        return out;
    }

    std::vector<std::string> motor_names() const {
        std::vector<std::string> out;
        out.reserve(motors.size());
        for (const auto& v : motors) out.push_back(v.name);
        return out;
    }

    std::unordered_map<int, size_t> joint_id_to_index() const {
        std::unordered_map<int, size_t> m;
        for (size_t i = 0; i < joints.size(); ++i) m[joints[i].ecat_id] = i;
        return m;
    }

    std::unordered_map<int, size_t> valve_id_to_index() const {
        std::unordered_map<int, size_t> m;
        for (size_t i = 0; i < valves.size(); ++i) m[valves[i].ecat_id] = i;
        return m;
    }

    std::unordered_map<int, size_t> motor_id_to_index() const {
        std::unordered_map<int, size_t> m;
        for (size_t i = 0; i < motors.size(); ++i) m[motors[i].ecat_id] = i;
        return m;
    }
};

inline std::optional<RobotConfig> load_robot_config(const std::string& yaml_path)
{
    try {
        YAML::Node root = YAML::LoadFile(yaml_path);

        RobotConfig cfg;
        cfg.robot_name = root["robot"]["name"].as<std::string>();
        cfg.domain_id  = root["domain"] ? root["domain"].as<uint32_t>() : 0u;

        if (root["joints"]) {
            for (const auto& j : root["joints"]) {
                JointConfig jc;
                jc.name    = j["name"].as<std::string>();
                jc.ecat_id = j["ecat_id"].as<int>();
                cfg.joints.push_back(jc);
            }
        }

        if (root["valves"]) {
            for (const auto& v : root["valves"]) {
                JointConfig vc;
                vc.name    = v["name"].as<std::string>();
                vc.ecat_id = v["ecat_id"].as<int>();
                cfg.valves.push_back(vc);
            }
        }

        if (root["motors"]) {
            for (const auto& v : root["motors"]) {
                JointConfig vc;
                vc.name    = v["name"].as<std::string>();
                vc.ecat_id = v["ecat_id"].as<int>();
                cfg.motors.push_back(vc);
            }
        }

        if (root["imu"]) {
            for (const auto& i : root["imu"])
                cfg.has_imu.push_back(i.as<std::string>());
        }

        if (root["power_board"]) {
            for (const auto& i : root["power_board"])
                cfg.has_power_board.push_back(i.as<std::string>());
        }

        if (root["force_torque"]) {
            for (const auto& i : root["force_torque"])
                cfg.has_force_torque.push_back(i.as<std::string>());
        }

        if (root["pump"]) {
            for (const auto& i : root["pump"])
                cfg.has_pump.push_back(i.as<std::string>());
        }

        return cfg;
    } catch (const YAML::Exception& e) {
        std::cerr << "[RobotConfig] Failed to parse '" << yaml_path << "': " << e.what() << '\n';
        return std::nullopt;
    }
}