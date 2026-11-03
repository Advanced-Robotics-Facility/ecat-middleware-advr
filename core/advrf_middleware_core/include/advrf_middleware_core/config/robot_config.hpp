#pragma once

#include <cstdint>
#include <limits>
#include <optional>
#include <set>
#include <string>
#include <unordered_map>

#include "advrf_middleware_core/config/config_file.hpp"
#include "advrf_middleware_core/utils/channel.hpp"
#include "advrf_middleware_core/utils/pdo_utils.hpp"

namespace advrf::middleware::config {

/// Transport-independent robot identity and EtherCAT device names.
struct RobotConfig {
    std::string robot_name;
    std::string ns;
    std::unordered_map<advrf::middleware::pdo::EcatId, std::string> map_ecat_id;
};

inline void merge_robot_identity(RobotConfig& cfg, const YAML::Node& root) {
    // Legacy files put robot identity under dds; new files use robot.
    const bool modern = static_cast<bool>(root["robot"]);
    const auto node = section(root, modern ? "robot" : "dds");
    const std::string path = modern ? "robot" : "dds";
    if (modern) check_keys(node, {"name", "namespace", "joint_map"}, "robot");
    cfg.robot_name = value<std::string>(node, modern ? "name" : "robot_name", cfg.robot_name, path);
    cfg.ns = value<std::string>(node, "namespace", cfg.ns, path);
}

inline void merge_robot_id_map(RobotConfig& cfg, const YAML::Node& root) {
    const bool modern = static_cast<bool>(root["robot"]);
    const auto parent = modern ? section(root, "robot") : root;
    const auto joints = section(parent, "joint_map", modern ? "robot" : "");
    const std::string path = modern ? "robot.joint_map" : "joint_map";
    for (const auto& joint : joints) {
        try {
            const auto key = joint.first.as<std::string>();
            YAML::Node entry;
            entry["id"] = key;
            const auto id = unsigned_value(entry, "id", 0, path);
            const auto name = joint.second.as<std::string>();
            if (id == 0 || name.empty()) throw ConfigError(path + ": IDs must be positive and names nonempty");
            if (id > static_cast<uint64_t>(std::numeric_limits<advrf::middleware::pdo::EcatId>::max()))
                throw ConfigError(path + ": EtherCAT ID out of range");
            cfg.map_ecat_id[static_cast<advrf::middleware::pdo::EcatId>(id)] = name;
        } catch (const YAML::Exception& e) {
            throw ConfigError(path + ": " + e.what());
        }
    }
}

inline void validate(const RobotConfig& cfg) {
    if (cfg.robot_name.empty()) throw ConfigError("robot.name: must not be empty");
    if (cfg.map_ecat_id.empty()) throw ConfigError("robot.joint_map: at least one device is required for discovery");
    if (cfg.robot_name.find('/') != std::string::npos)
        throw ConfigError("robot.name: must be a single topic component");
}

inline RobotConfig parse_robot_config(const YAML::Node& root) {
    RobotConfig cfg;
    merge_robot_identity(cfg, root);
    merge_robot_id_map(cfg, root);
    validate(cfg);
    return cfg;
}

// EC_CFG selects the authoritative joint map; standalone YAML remains supported.
inline RobotConfig parse_runtime_robot_config(const YAML::Node& root) {
    const auto* env = std::getenv("EC_CFG");
    if (!env) return parse_robot_config(root);
    RobotConfig cfg;
    merge_robot_identity(cfg, root);
    const auto file = ConfigFile::load(robot_id_map_path());
    merge_robot_id_map(cfg, file.root);
    validate(cfg);
    return cfg;
}

inline void load_from_ecat_config(RobotConfig& cfg, const std::string& path) {
    auto next = cfg;
    merge_robot_identity(next, ConfigFile::load(path).root);
    cfg = std::move(next);
}
inline void load_from_robot_id_map(RobotConfig& cfg, const std::string& path) {
    auto next = cfg;
    merge_robot_id_map(next, ConfigFile::load(path).root);
    cfg = std::move(next);
}
inline std::optional<RobotConfig> load_robot_config(const std::string& map_path,
                                                   const std::string& identity_path) {
    try {
        if (map_path == identity_path) return parse_runtime_robot_config(ConfigFile::load(map_path).root);
        RobotConfig cfg;
        load_from_robot_id_map(cfg, map_path);
        load_from_ecat_config(cfg, identity_path);
        validate(cfg);
        return cfg;
    } catch (const ConfigError&) { return std::nullopt; }
}

inline std::set<uint32_t> extract_pdo_ids(const RobotConfig& cfg) {
    std::set<uint32_t> ids;
    for (const auto& [id, name] : cfg.map_ecat_id) ids.insert(id);
    return ids;
}

class RobotConfigBuilder {
public:
    RobotConfigBuilder& from_ecat_config(const std::string& path) {
        load_from_ecat_config(config_, path); return *this;
    }
    RobotConfigBuilder& from_robot_id_map(const std::string& path) {
        load_from_robot_id_map(config_, path); return *this;
    }
    RobotConfig build() const { validate(config_); return config_; }
private:
    RobotConfig config_;
};

} // namespace advrf::middleware::config
