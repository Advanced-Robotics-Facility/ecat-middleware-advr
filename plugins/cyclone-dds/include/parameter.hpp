#pragma once

#include <stdexcept>
#include <string>
#include <vector>

#include <rcl_interfaces/msg/Parameter.hpp>
#include <rcl_interfaces/msg/ParameterValue.hpp>
#include <rcl_interfaces/msg/ParameterType.hpp>

using namespace rcl_interfaces::msg::dds_;
template<typename T> rcl_interfaces::msg::dds_::ParameterValue_ make_parameter_value(const T& value)
{
    using U = std::decay_t<T>;

    rcl_interfaces::msg::dds_::ParameterValue_ msg;

    if constexpr (std::is_same_v<U, bool>)
    {
        msg.type(PARAMETER_BOOL);
        msg.bool_value(value);
    }
    else if constexpr (std::is_integral_v<U> && !std::is_same_v<U, bool>)
    {
        msg.type(PARAMETER_INTEGER);
        msg.integer_value(static_cast<int64_t>(value));
    }
    else if constexpr (std::is_floating_point_v<U>)
    {
        msg.type(PARAMETER_DOUBLE);
        msg.double_value(static_cast<double>(value));
    }
    else if constexpr (std::is_same_v<U, std::string>)
    {
        msg.type(PARAMETER_STRING);
        msg.string_value(value);
    }
    else if constexpr (std::is_same_v<U, std::vector<uint8_t>>)
    {
        msg.type(PARAMETER_BYTE_ARRAY);
        msg.byte_array_value(value);
    }
    else if constexpr (std::is_same_v<U, std::vector<bool>>)
    {
        msg.type(PARAMETER_BOOL_ARRAY);
        msg.bool_array_value(value);
    }
    else if constexpr (std::is_same_v<U, std::vector<int64_t>>)
    {
        msg.type(PARAMETER_INTEGER_ARRAY);
        msg.integer_array_value(value);
    }
    else if constexpr (std::is_same_v<U, std::vector<double>>)
    {
        msg.type(PARAMETER_DOUBLE_ARRAY);
        msg.double_array_value(value);
    }
    else if constexpr (std::is_same_v<U, std::vector<std::string>>)
    {
        msg.type(PARAMETER_STRING_ARRAY);
        msg.string_array_value(value);
    }
    else
    {
        static_assert(!sizeof(U), "Unsupported type for make_parameter().");
    }

    return msg;
}




template<typename T>
T get_parameter_value(const rcl_interfaces::msg::dds_::ParameterValue_& msg)
{
    using U = std::decay_t<T>;

    if constexpr (std::is_same_v<U, bool>)
    {
        if (msg.type() != PARAMETER_BOOL)
            throw std::runtime_error("Parameter is not a bool.");
        return msg.bool_value();
    }
    else if constexpr (std::is_integral_v<U> && !std::is_same_v<U, bool>)
    {
        if (msg.type() != PARAMETER_INTEGER)
            throw std::runtime_error("Parameter is not an integer.");
        return static_cast<U>(msg.integer_value());
    }
    else if constexpr (std::is_floating_point_v<U>)
    {
        if (msg.type() != PARAMETER_DOUBLE)
            throw std::runtime_error("Parameter is not a double.");
        return static_cast<U>(msg.double_value());
    }
    else if constexpr (std::is_same_v<U, std::string>)
    {
        if (msg.type() != PARAMETER_STRING)
            throw std::runtime_error("Parameter is not a string.");
        return msg.string_value();
    }
    else if constexpr (std::is_same_v<U, std::vector<uint8_t>>)
    {
        if (msg.type() != PARAMETER_BYTE_ARRAY)
            throw std::runtime_error("Parameter is not a byte array.");
        return msg.byte_array_value();
    }
    else if constexpr (std::is_same_v<U, std::vector<bool>>)
    {
        if (msg.type() != PARAMETER_BOOL_ARRAY)
            throw std::runtime_error("Parameter is not a bool array.");
        return msg.bool_array_value();
    }
    else if constexpr (std::is_same_v<U, std::vector<int64_t>>)
    {
        if (msg.type() != PARAMETER_INTEGER_ARRAY)
            throw std::runtime_error("Parameter is not an integer array.");
        return msg.integer_array_value();
    }
    else if constexpr (std::is_same_v<U, std::vector<double>>)
    {
        if (msg.type() != PARAMETER_DOUBLE_ARRAY)
            throw std::runtime_error("Parameter is not a double array.");
        return msg.double_array_value();
    }
    else if constexpr (std::is_same_v<U, std::vector<std::string>>)
    {
        if (msg.type() != PARAMETER_STRING_ARRAY)
            throw std::runtime_error("Parameter is not a string array.");
        return msg.string_array_value();
    }
    else
    {
        static_assert(!sizeof(U), "Unsupported type for get_parameter().");
    }
}


template<typename T>
rcl_interfaces::msg::dds_::Parameter_
make_parameter(const std::string& name, const T& value)
{
    rcl_interfaces::msg::dds_::Parameter_ parameter;

    parameter.name(name);
    parameter.value(make_parameter_value(value));

    return parameter;
}

template<typename T>
T get_parameter(const rcl_interfaces::msg::dds_::Parameter_& parameter)
{
    return get_parameter_value<T>(parameter.value());
}