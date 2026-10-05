#pragma once
#include <string>
#include <unordered_map>
#include <cstdint>

/**
 * @brief Resolve a device name from an identifier ending in an EtherCAT ID.
 *
 * Extracts the numeric suffix after the final underscore in @p str_id and
 * looks it up in @p id_to_name.
 *
 * @param str_id Original identifier, for example `motor_12`.
 * @param id_to_name Mapping from EtherCAT IDs to configured device names.
 * @return The mapped device name if a valid matching ID is found; otherwise
 *         the original @p str_id unchanged.
 */
inline std::string resolve_name(const std::string& str_id,
                                const std::unordered_map<uint32_t, 
                                std::string>& id_to_name)
{
    const auto pos = str_id.rfind('_');
    if (pos == std::string::npos) return str_id;

    try {
        const auto id = static_cast<uint32_t>(std::stoi(str_id.substr(pos + 1)));
        if (auto it = id_to_name.find(id); it != id_to_name.end())
            return it->second;
    } catch (...) {}

    return str_id;
}