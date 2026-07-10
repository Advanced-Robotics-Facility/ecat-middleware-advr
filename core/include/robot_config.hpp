#pragma once

#include <cstdint>
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
    std::string robot_name {"NoNe"};
    uint32_t domain_id {0};

    std::vector<JointConfig> joints;  
    std::vector<JointConfig> motors;   
    std::vector<JointConfig> valves;  
    std::vector<JointConfig> grippers; 

    std::vector<std::string> motor_names() const { return get_names(motors); }
    std::vector<std::string> joint_names() const { return get_names(joints); }
    std::vector<std::string> valve_names() const { return get_names(valves); }
    std::vector<std::string> gripper_names() const { return get_names(grippers); }

    std::unordered_map<int, size_t> motor_id_to_index() const { return get_map(motors); }
    std::unordered_map<int, size_t> joint_id_to_index() const { return get_map(joints); }
    std::unordered_map<int, size_t> valve_id_to_index() const { return get_map(valves); }
    std::unordered_map<int, size_t> gripper_id_to_index() const { return get_map(grippers); }

    private:

        static std::vector<std::string> get_names(const std::vector<JointConfig>& vec) {
            std::vector<std::string> out;
            for (const auto& j : vec) 
                out.push_back(j.name);
            return out;
        }
    
        static std::unordered_map<int, size_t> get_map(const std::vector<JointConfig>& vec) {
            std::unordered_map<int, size_t> m;
            for (size_t i = 0; i < vec.size(); ++i) 
                m[vec[i].ecat_id] = i;
            return m;
        }
};

inline std::optional<RobotConfig> load_robot_config(const std::string& yaml_path)
{
    try {
        YAML::Node root = YAML::LoadFile(yaml_path);
        YAML::Node robot = root["robot"] ? root["robot"] : root;
        YAML::Node dds = root["dds"];

        RobotConfig cfg;
        cfg.robot_name = robot["name"] ? robot["name"].as<std::string>() : "NoNe";
        cfg.domain_id  = dds && dds["domain"] ? dds["domain"].as<uint32_t>() :
                         robot["domain"] ? robot["domain"].as<uint32_t>() :
                         root["domain"] ? root["domain"].as<uint32_t>() :
                         0u;

        const YAML::Node joints = robot["joints"] ? robot["joints"] : root["joints"];
        if (joints) {
            for (const auto& j : joints) {
                JointConfig jc;
                jc.name    = j["name"].as<std::string>();
                jc.ecat_id = j["ecat_id"].as<int>();

                std::string type = j["type"] ? j["type"].as<std::string>() : "motor";

                cfg.joints.push_back(jc);

                if (type == "motor")
                    cfg.motors.push_back(jc);
                else if (type == "gripper")
                    cfg.grippers.push_back(jc);
                else
                    cfg.valves.push_back(jc); 
            }
        }

        return cfg;
    } catch (const YAML::Exception& e) {
        std::cerr << "[RobotConfig] Failed to parse '" << yaml_path << "': " << e.what() << '\n';
        return std::nullopt;
    }
}
