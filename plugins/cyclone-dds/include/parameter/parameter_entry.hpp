#pragma once

#include <functional>
#include <string>

#include <rcl_interfaces/msg/Parameter.hpp>

/**
 * @brief Options for a parameter.
 */
template <typename T>
struct ParameterOptions
{
    std::string description;
    bool read_only = false;
    bool dynamic = true;
    std::function<bool(const T&&)> validator;
};

/**
 * @brief Entry for a parameter.
 */
struct ParameterEntry
{
    rcl_interfaces::msg::dds_::Parameter_ parameter;
    std::string description;
    bool read_only{false};
    bool dynamic{true};
    std::function<bool(const rcl_interfaces::msg::dds_::Parameter_&)> validator;
};