#pragma once

#include <cstdint>
#include <filesystem>
#include <initializer_list>
#include <stdexcept>
#include <string>
#include <yaml-cpp/yaml.h>

namespace advrf::middleware::config {

class ConfigError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

inline YAML::Node section(const YAML::Node& parent, const std::string& key,
                          const std::string& path = "") {
    const auto node = parent[key];
    if (node && !node.IsMap())
        throw ConfigError((path.empty() ? key : path + "." + key) + ": expected a mapping");
    return node ? node : YAML::Node(YAML::NodeType::Map);
}

inline void check_keys(const YAML::Node& node, std::initializer_list<const char*> allowed,
                       const std::string& path) {
    for (const auto& entry : node) {
        const auto key = entry.first.as<std::string>();
        bool known = false;
        for (const auto* candidate : allowed) if (key == candidate) { known = true; break; }
        if (!known) throw ConfigError(path + "." + key + ": unknown setting");
    }
}

template<class T>
inline T value(const YAML::Node& parent, const std::string& key, T fallback,
               const std::string& path) {
    const auto node = parent[key];
    if (!node) return fallback;
    try { return node.as<T>(); }
    catch (const YAML::Exception& e) {
        throw ConfigError(path + "." + key + ": " + e.what());
    }
}

// Parse unsigned integers explicitly: yaml-cpp conversions may accept negatives.
inline uint32_t unsigned_value(const YAML::Node& parent, const std::string& key,
                               uint32_t fallback, const std::string& path) {
    const auto text = value<std::string>(parent, key, std::to_string(fallback), path);
    if (text.empty() || text.find_first_not_of("0123456789") != std::string::npos)
        throw ConfigError(path + "." + key + ": expected an unsigned integer");
    try {
        const auto number = std::stoull(text);
        if (number > UINT32_MAX) throw std::out_of_range("uint32");
        return static_cast<uint32_t>(number);
    } catch (const std::exception&) {
        throw ConfigError(path + "." + key + ": integer out of range");
    }
}

struct ConfigFile {
    YAML::Node root;
    std::filesystem::path path;

    static ConfigFile load(const std::filesystem::path& path) {
        try {
            auto root = YAML::LoadFile(path.string());
            if (!root.IsMap()) throw ConfigError("expected a YAML mapping");
            if (unsigned_value(root, "version", 1, "config") != 1)
                throw ConfigError("config.version: only version 1 is supported");
            return {root, std::filesystem::absolute(path)};
        } catch (const std::exception& e) {
            throw ConfigError(path.string() + ": " + e.what());
        }
    }

    std::filesystem::path resolve(const std::string& reference) const {
        const std::filesystem::path target(reference);
        return target.is_absolute() ? target : (path.parent_path() / target).lexically_normal();
    }
};

} // namespace advrf::middleware::config
