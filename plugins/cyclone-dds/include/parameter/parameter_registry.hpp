#pragma once

#include <stdexcept>
#include <string>
#include <unordered_map>

#include "parameter/parameter_make_get.hpp"
#include "parameter/parameter_entry.hpp"

#include <advrf_interfaces/msg/Enums.hpp>

// #include "parameter/parameter_result.hpp"

/**
 * @brief Registry for managing parameters.
 */
class ParameterRegistry
{
public:

    using ParameterResult = advrf_interfaces::msg::dds_::enums_::ParameterResult_;
    ParameterRegistry() = default;

    //
    // Parameter declaration
    //

   template<class T>
    ParameterResult declare(
        const std::string& name,
        const T& value,
        const ParameterOptions<T>& options = {})
    {
        if (has(name))
            return ParameterResult::AlreadyDeclared;

        ParameterEntry entry;

        entry.parameter   = make_parameter(name, value);
        entry.description = options.description;
        entry.read_only   = options.read_only;
        entry.dynamic     = options.dynamic;
        entry.validator   = [options](const rcl_interfaces::msg::dds_::Parameter_& parameter)
        {
             if (!options.validator) {
                return true;
            }
            return options.validator(get_parameter<T>(parameter));
        };

        parameters_.emplace(name, std::move(entry));
        return ParameterResult::Success;
    }

    //
    // Parameter update
    //

    ParameterResult set(
        const rcl_interfaces::msg::dds_::Parameter_& parameter)
    {
        auto it = parameters_.find(parameter.name());

        if (it == parameters_.end())
            return ParameterResult::NotFound;

        if (it->second.read_only)
            return ParameterResult::ReadOnly;

        if (it->second.validator)
        {
            if (!it->second.validator(parameter))
                return ParameterResult::ValidationFailed;
        }

        it->second.parameter = parameter;

        return ParameterResult::Success;
    }

    template<class T>
    ParameterResult set(
        const std::string& name,
        const T& value)
    {
        return set(make_parameter(name, value));
    }

    std::vector<ParameterResult> set(const std::vector<rcl_interfaces::msg::dds_::Parameter_>& parameters)
    {
        std::vector<ParameterResult> results;
        results.reserve(parameters.size());

        for (const auto& parameter : parameters)
        {
            results.emplace_back(set(parameter));
        }

        return results;
    }

    
    //
    // Parameter access
    //

    std::vector<rcl_interfaces::msg::dds_::Parameter_> get(const std::vector<std::string>& names) const
    {
        std::vector<rcl_interfaces::msg::dds_::Parameter_> parameters;
        parameters.reserve(names.size());

        for (const auto& name : names)
        {
            auto it = parameters_.find(name);

            if (it != parameters_.end())
            {
                parameters.emplace_back(it->second.parameter);
            }
        }

        return parameters;
    }

    // std::vector<rcl_interfaces::msg::dds_::Parameter_> get() const 
    // {
    //     std::vector<rcl_interfaces::msg::dds_::Parameter_> parameters;
    //     parameters.reserve(parameters_.size());

    //     for (const auto& [name, entry] : parameters_)
    //     {
    //         parameters.emplace_back(entry.parameter);
    //     }

    //     return parameters;
    // }

    template<class T> T get(const std::string& name) const
    {
        return get_parameter<T>(at(name).parameter);
    }

    const rcl_interfaces::msg::dds_::Parameter_& get(const std::string& name) const
    {
        return at(name).parameter;
    }

    std::vector<rcl_interfaces::msg::dds_::Parameter_> get() const
    {
        std::vector<rcl_interfaces::msg::dds_::Parameter_> parameters;
        parameters.reserve(parameters_.size());

        for (const auto& [name, entry] : parameters_)
        {
            parameters.emplace_back(entry.parameter);
        }

        return parameters;
    }

    //
    // Entry access
    //

    ParameterEntry&
    at(const std::string& name)
    {
        auto it = parameters_.find(name);

        if (it == parameters_.end())
            throw std::runtime_error("Unknown parameter '" + name + "'.");

        return it->second;
    }

    const ParameterEntry&
    at(const std::string& name) const
    {
        auto it = parameters_.find(name);

        if (it == parameters_.end())
            throw std::runtime_error("Unknown parameter '" + name + "'.");

        return it->second;
    }

    //
    // Utilities
    //

    bool has(const std::string& name) const
    {
        return parameters_.find(name) != parameters_.end();
    }

    ParameterResult remove(const std::string& name)
    {
        auto it = parameters_.find(name);

        if (it == parameters_.end())
            return ParameterResult::NotFound;

        parameters_.erase(it);

        return ParameterResult::Success;
    }

    void clear()
    {
        parameters_.clear();
    }

    std::size_t size() const
    {
        return parameters_.size();
    }

    //
    // Iterators
    //

    auto begin() noexcept
    {
        return parameters_.begin();
    }

    auto end() noexcept
    {
        return parameters_.end();
    }

    auto begin() const noexcept
    {
        return parameters_.begin();
    }

    auto end() const noexcept
    {
        return parameters_.end();
    }

    //
    // Direct access
    //

    const std::unordered_map<std::string, ParameterEntry>&
    parameters() const
    {
        return parameters_;
    }

    std::vector<std::string> list() const
    {
        std::vector<std::string> names;
        names.reserve(parameters_.size());
        for (const auto& [name, entry] : parameters_)
        {
            names.emplace_back(name);
        }
        std::sort(names.begin(), names.end());

        return names;
    }

    std::vector<std::string> list(const std::string& prefix) const
    {
        std::vector<std::string> names;
        for (const auto& [name, entry] : parameters_)
        {
            if (name.rfind(prefix, 0) == 0)
            {
                names.emplace_back(name);
            }
        }
        std::sort(names.begin(), names.end());
        return names;
    }

private:

    std::unordered_map<std::string, ParameterEntry> parameters_;
};