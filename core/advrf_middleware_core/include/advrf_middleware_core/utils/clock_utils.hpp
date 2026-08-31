#pragma once

#include <cstdint>
#include <ctime>
#include <iostream>

namespace clock_utils {

/**
 * @brief Sampled offset between CLOCK_REALTIME and CLOCK_MONOTONIC.
 */
struct ClockOffset {
    uint64_t realtime_ns  = 0; 
    uint64_t monotonic_ns = 0;
    uint64_t K = 0; 
};
inline ClockOffset g_offset;

/**
 * @brief Sample realtime and monotonic clocks and calculate their offset.
 *
 * Call once during initialization, before monotonic_to_realtime().
 */
inline void init()
{
    struct timespec rt{};
    clock_gettime(CLOCK_REALTIME,  &rt);

    struct timespec mono{};
    clock_gettime(CLOCK_MONOTONIC, &mono);

    g_offset.realtime_ns = static_cast<uint64_t>(rt.tv_sec) * 1'000'000'000ULL + static_cast<uint64_t>(rt.tv_nsec);
    g_offset.monotonic_ns = static_cast<uint64_t>(mono.tv_sec) * 1'000'000'000ULL + static_cast<uint64_t>(mono.tv_nsec);
    g_offset.K = g_offset.realtime_ns - g_offset.monotonic_ns;
}


/**
 * @brief Convert a monotonic timestamp to an estimated Unix-epoch timestamp.
 *
 * The conversion uses the offset sampled by init().
 */
inline uint64_t monotonic_to_realtime(uint64_t mono_ns)
{
    return mono_ns + g_offset.K;
}

/**
 * @brief Return the current CLOCK_MONOTONIC time in nanoseconds.
 *
 * Used when an incoming PDO has no timestamp.
 */
inline uint64_t monotonic_now_ns()
{
    struct timespec ts{};
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return static_cast<uint64_t>(ts.tv_sec) * 1'000'000'000ULL + static_cast<uint64_t>(ts.tv_nsec);
}

} 