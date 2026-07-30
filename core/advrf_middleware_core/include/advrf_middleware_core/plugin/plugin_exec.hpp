#pragma once

#include <atomic>
#include <chrono>
#include <memory>
#include <string>
#include <thread>
#include <csignal>

#include "advrf_middleware_core/adapters/adapter_base.hpp"
#include "advrf_middleware_core/utils/log.hpp"

namespace advrf::plugin
{

class Process
{
public:
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
            auto next_health = next + std::chrono::seconds(1);q
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

class PluginExec
{
public:
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

    void register_adapter(Process process)
    {
        processes_.push_back(std::move(process));
    }

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

    void stop()
    {
        if (!running_.exchange(false))
            return;

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