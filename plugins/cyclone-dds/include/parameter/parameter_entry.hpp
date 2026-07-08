#pragma once

#include <functional>
#include <string>

#include <rcl_interfaces/msg/Parameter.hpp>

struct ParameterEntry
{
    rcl_interfaces::msg::dds_::Parameter_ parameter;
    std::string description;
    bool read_only{false};
    bool dynamic{true};
    std::function<bool(const rcl_interfaces::msg::dds_::Parameter_&)>
        validator;
};