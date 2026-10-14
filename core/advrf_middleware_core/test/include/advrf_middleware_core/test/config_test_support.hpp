#pragma once
#include <stdexcept>
#include <string>

#include "advrf_middleware_core/config/config_file.hpp"

inline void require(bool ok, const std::string& message) {
    if (!ok) throw std::runtime_error(message);
}
template<class Fn>
inline void expect_error(Fn action, const std::string& field) {
    try { action(); }
    catch (const advrf::middleware::config::ConfigError& e) {
        require(std::string(e.what()).find(field) != std::string::npos,
                "Expected error mentioning " + field + ", got " + e.what());
        return;
    }
    throw std::runtime_error("Expected configuration error: " + field);
}
