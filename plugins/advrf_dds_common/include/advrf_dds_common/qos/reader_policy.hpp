#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string>

namespace advrf::dds_common {

enum class Reliability
{
    BestEffort,
    Reliable
};

enum class Durability
{
    Volatile,
    TransientLocal,
    Transient,
    Persistent
};

struct ReaderPolicy
{
    Reliability reliability{Reliability::BestEffort};
    Durability durability{Durability::Volatile};
    std::size_t history_depth{1};

    constexpr bool is_valid() const noexcept
    {
        return history_depth > 0 &&
               history_depth <= static_cast<std::size_t>(
                   std::numeric_limits<std::int32_t>::max());
    }
};


class ReaderPolicyBuilder
{
public:
    ReaderPolicyBuilder() = default;    

    ReaderPolicyBuilder& set_reliability(Reliability reliability)
    {
        policy_.reliability = reliability;
        return *this;
    }

    ReaderPolicyBuilder& set_reliability(const std::string& reliability_str)
    {
        if (reliability_str == "best_effort") {
            policy_.reliability = Reliability::BestEffort;
        } else if (reliability_str == "reliable") {
            policy_.reliability = Reliability::Reliable;
        } else {
            throw std::invalid_argument("Invalid reliability string: " + reliability_str);
        }
        return *this;
    }
    
    ReaderPolicyBuilder& set_durability(Durability durability)
    {
        policy_.durability = durability;
        return *this;
    }

    ReaderPolicyBuilder& set_durability(const std::string& durability_str){
        
        if (durability_str == "volatile") {
            policy_.durability = Durability::Volatile;
        } else if (durability_str == "transient_local") {
            policy_.durability = Durability::TransientLocal;
        } else if (durability_str == "transient") {
            policy_.durability = Durability::Transient;
        } else if (durability_str == "persistent") {
            policy_.durability = Durability::Persistent;
        } else {
            throw std::invalid_argument("Invalid durability string: " + durability_str);
        }
        return *this;
    }

    ReaderPolicyBuilder& set_history_depth(std::size_t history_depth)
    {   
        policy_.history_depth = history_depth;
        return *this;
    }

    ReaderPolicy build() const
    {
        return policy_;
    }


private:
    ReaderPolicy policy_;


};


} // namespace advrf::dds_common
