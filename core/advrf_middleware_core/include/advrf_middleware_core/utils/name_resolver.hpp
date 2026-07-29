#pragma once
#include <string>
#include <unordered_map>
#include <cstdint>

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