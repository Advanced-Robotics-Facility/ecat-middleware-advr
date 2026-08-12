#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>

namespace advrf::dds_common {

enum class Reliability
{
    BestEffort,
    Reliable
};

struct ReaderPolicy
{
    Reliability reliability{Reliability::BestEffort};
    std::size_t history_depth{1};

    constexpr bool is_valid() const noexcept
    {
        return history_depth > 0 &&
               history_depth <= static_cast<std::size_t>(
                   std::numeric_limits<std::int32_t>::max());
    }
};

}
