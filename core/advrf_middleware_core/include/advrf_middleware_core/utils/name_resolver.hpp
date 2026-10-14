#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>

/** Resolve a configured device name from its EtherCAT numeric ID. */
inline std::string resolve_name(
    std::uint32_t id,
    const std::unordered_map<std::uint32_t, std::string>& names)
{
    if (auto it = names.find(id); it != names.end())
        return it->second;

    return "ecat_" + std::to_string(id);
}
