#pragma once

#include <cstdint>
#include <ctime>

namespace advrf::middleware::clock {

/** @brief Return the current Linux CLOCK_MONOTONIC time in nanoseconds. */
inline uint64_t monotonic_now_ns()
{
    struct timespec ts{};
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return static_cast<uint64_t>(ts.tv_sec) * 1'000'000'000ULL +
           static_cast<uint64_t>(ts.tv_nsec);
}

/**
 * @brief Convert a Linux monotonic sample to Unix-epoch time.
 */
inline uint64_t monotonic_to_realtime(uint64_t mono_ns)
{
    const auto before_ns = monotonic_now_ns();
    struct timespec rt{};
    clock_gettime(CLOCK_REALTIME, &rt);
    const auto after_ns = monotonic_now_ns();
    const auto midpoint_ns = before_ns + (after_ns - before_ns) / 2;
    const auto realtime_ns = static_cast<uint64_t>(rt.tv_sec) * 1'000'000'000ULL +
                             static_cast<uint64_t>(rt.tv_nsec);
    return realtime_ns + mono_ns - midpoint_ns;
}

}