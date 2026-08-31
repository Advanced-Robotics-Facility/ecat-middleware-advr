#pragma once

#include <atomic>
#include <chrono>
#include <memory>
#include <vector>
#include <string>
#include <thread>
#include <csignal>

#include "advrf_middleware_core/adapters/adapter_base.hpp"
#include "advrf_middleware_core/utils/log.hpp"

namespace advrf::plugin
{

/**
 * @brief Runs one middleware adapter in a periodic worker thread.
 *
 * The adapter is started once, then @ref AdapterBase::spin_once is called at
 * the configured period until the shared @p running flag becomes false.
 */
class Process
{
public:
    /**
     * @param name Name used in log messages.
     * @param adapter Adapter owned and executed by this process.
     * @param period Desired execution period.
     */
    Process(std::string name,
            std::shared_ptr<AdapterBase> adapter,
            std::chrono::microseconds period = std::chrono::microseconds{10})
        : name_(std::move(name))
        , adapter_(std::move(adapter))
        , period_(period)
    {
    }

    Process(const Process&) = delete;
    Process& operator=(const Process&) = delete;

    Process(Process&&) = default;
    Process& operator=(Process&&) = default;

    ~Process()
    {
        stop();
    }

    /**
     * @brief Start the adapter and its periodic worker thread.
     *
     * @param running Shared application-running flag.
     * @return False if the adapter is null or cannot be started.
     */
    bool start(std::atomic_bool& running)
    {
        if (!adapter_)
        {
            LOG_ERROR("Adapter '{}' is null.", name_);
            return false;
        }

        if (!adapter_->start())
        {
            LOG_ERROR("Adapter '{}' failed to start.", name_);
            return false;
        }

        thread_ = std::thread([this, &running]
        {
            LOG_INFO("Adapter '{}' started ({} Hz).",
                     name_,
                     1'000'000 / period_.count());

            auto next = std::chrono::steady_clock::now();
            auto next_health = next + std::chrono::seconds(1);
            while (running)
            {
                next += period_;
                if (std::chrono::steady_clock::now() >= next_health)
                {
                    if (!adapter_->is_ok())
                    {
                        LOG_ERROR("Adapter '{}' stopped unexpectedly.", name_);
                        running = false;
                        break;
                    }
                    next_health += std::chrono::seconds(1);
                }
                adapter_->spin_once();
                std::this_thread::sleep_until(next);
            }
        });

        return true;
    }

    /**
     * @brief Wait for the worker thread to terminate.
     *
     * The caller must first set the shared running flag to false.
     */
    void stop()
    {
        if (thread_.joinable())
            thread_.join();
    }

private:
    std::string name_;
    std::shared_ptr<AdapterBase> adapter_;
    std::chrono::microseconds period_;
    std::thread thread_;
};

} // namespace advrf::plugin

namespace advrf::plugin
{

/**
 * @brief Coordinates the lifecycle of multiple middleware adapter processes.
 *
 * Installs SIGINT and SIGTERM handlers. Receiving either signal requests all
 * registered processes to stop.
 */
class PluginExec
{
public:
    /// Install termination-signal handlers.
    PluginExec()
    {
        instance_ = this;

        std::signal(SIGINT, SignalHandler);
        std::signal(SIGTERM, SignalHandler);
    }

    ~PluginExec()
    {
        stop();
        instance_ = nullptr;
    }

    /**
     * @brief Register an adapter process before starting the application.
     */
    void register_adapter(Process process)
    {
        processes_.push_back(std::move(process));
    }

    /**
     * @brief Start all registered processes.
     *
     * @param wait_for_exit If true, block until a termination signal or an
     *                      adapter-health failure stops the application.
     */
    void start(bool wait_for_exit = true)
    {
        LOG_INFO("Application started (Ctrl+C to exit).");

        running_ = true;

        for (auto& process : processes_)
        {
            if (!process.start(running_))
            {
                running_ = false;
                break;
            }
        }

        if (!wait_for_exit)
            return;

        while (running_)
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

        stop();

        LOG_INFO("Application exited.");
    }

    /**
     * @brief Request termination and join all worker threads.
     */
    void stop()
    {
        running_ = false;

        for (auto& process : processes_)
            process.stop();
    }

private:
    static void SignalHandler(int) noexcept
    {
        if (instance_)
            instance_->running_ = false;
    }

    inline static PluginExec* instance_{nullptr};

    std::atomic_bool running_{false};
    std::vector<Process> processes_;
};

} // namespace advrf::plugin