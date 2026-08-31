#pragma once

#include <mutex>
#include <spdlog/spdlog.h>

namespace advrf::log
{

    /**
     * @brief One-time initialization of the project's spdlog configuration.
     *
     * The configured log level follows @c SPDLOG_ACTIVE_LEVEL. Log messages
     * uses a timestamp, colourized console format. 
     */
    class Log
    {
    public:
        /**
         * @brief Initialize spdlog once in a thread-safe manner.
         *
         * Repeated calls have no effect.
         */
        static inline void init()
        {
            static std::once_flag once;
            std::call_once(once, []()
            {
            #if SPDLOG_ACTIVE_LEVEL <= SPDLOG_LEVEL_TRACE
                        spdlog::set_level(spdlog::level::trace);
            #elif SPDLOG_ACTIVE_LEVEL <= SPDLOG_LEVEL_DEBUG
                        spdlog::set_level(spdlog::level::debug);
            #elif SPDLOG_ACTIVE_LEVEL <= SPDLOG_LEVEL_INFO
                        spdlog::set_level(spdlog::level::info);
            #elif SPDLOG_ACTIVE_LEVEL <= SPDLOG_LEVEL_WARN
                        spdlog::set_level(spdlog::level::warn);
            #elif SPDLOG_ACTIVE_LEVEL <= SPDLOG_LEVEL_ERROR
                        spdlog::set_level(spdlog::level::err);
            #else
                        spdlog::set_level(spdlog::level::critical);
            #endif

                spdlog::set_pattern("[%H:%M:%S.%e] [%^%l%$] %v");
            });
        }

    private:
        Log() = delete;
    };

} // namespace advrf::log

/**
 * @brief Convenience macros forwarding to spdlog at the corresponding level.
 */
#define LOG_TRACE(...)    SPDLOG_TRACE(__VA_ARGS__)
#define LOG_DEBUG(...)    SPDLOG_DEBUG(__VA_ARGS__)
#define LOG_INFO(...)     SPDLOG_INFO(__VA_ARGS__)
#define LOG_WARN(...)     SPDLOG_WARN(__VA_ARGS__)
#define LOG_ERROR(...)    SPDLOG_ERROR(__VA_ARGS__)
#define LOG_CRITICAL(...) SPDLOG_CRITICAL(__VA_ARGS__)