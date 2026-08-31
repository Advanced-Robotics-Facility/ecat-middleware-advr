#pragma once

/**
 * @brief Common lifecycle interface for middleware adapters.
 */
class AdapterBase {
    public:
        virtual ~AdapterBase() = default;

        /// Connect and initialize the adapter.
        virtual bool start() = 0;

        /// Process one middleware cycle.
        virtual void spin_once() = 0;

        /// Check whether the adapter connection is usable.
        virtual bool is_ok() const = 0;

        /// Release adapter resources and disconnect.
        virtual void close() = 0;
};