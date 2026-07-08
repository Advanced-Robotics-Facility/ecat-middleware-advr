#pragma once

#include <functional>
#include <string>

#include <rcl_interfaces/msg/Parameter.hpp>

/**
 * @brief Options for a parameter.
 */
struct ParameterOptions
{
    std::string description;
    bool read_only = false;
    bool dynamic = true;
    std::function<bool(const rcl_interfaces::msg::dds_::Parameter_&)> validator;
};