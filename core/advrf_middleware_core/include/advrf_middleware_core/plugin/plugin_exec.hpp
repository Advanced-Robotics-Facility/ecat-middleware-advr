#pragma once

#include <atomic>
#include <chrono>
#include <csignal>
#include <memory>
#include <thread>
#include <vector>

#include "advrf_middleware_core/adapters/adapter_base.hpp"
#include "advrf_middleware_core/utils/log.hpp"

namespace advrf::plugin {

struct Process
{
    std::weak_ptr<AdapterBase> adapter;
    std::chrono::microseconds period_microseconds{10};
};

class PluginExec
{
public:
    PluginExec()
    {
        running_ = true;
        std::signal(SIGINT, SignalHandler);
        std::signal(SIGTERM, SignalHandler);
    }

    virtual ~PluginExec()
    {
        stop();
    }

    void register_adapter(const Process& process)
    {
        processes_.push_back(process);
    }

    void start(bool wait_for_exit = true)
    {
        for (auto& process : processes_)
        {
            if (auto adapter = process.adapter.lock())
            {
                threads_.emplace_back([adapter, period = process.period_microseconds]()
                {
                    auto next = std::chrono::steady_clock::now();
                    while (running_)
                    {
                        next += period;
                        auto start = std::chrono::steady_clock::now();
                        adapter->spin_once();
                        auto end = std::chrono::steady_clock::now();
                        std::chrono::duration<double, std::milli> elapsed = end - start;
                        std::this_thread::sleep_until(next);
                    }
                });
            }
            else{
                LOG_ERROR("Adapter is not valid. Skipping.");
            }
        }

        if(threads_.size() < processes_.size()){
            LOG_ERROR("Some adapters were not valid. Check previous error messages.");
        }

        if (wait_for_exit)
        {
            LOG_INFO("Application Run (Kill by CTRL+C)");
            while (running_)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            LOG_INFO("Application Exit");
        }
    }

    void stop()
    {
        running_ = false;

        for (auto& thread : threads_)
        {
            if (thread.joinable())
                thread.join();
        }

        threads_.clear();
    }

private:
    static void SignalHandler(int)
    {
        running_ = false;
    }

    static inline std::atomic_bool running_{true};

    std::vector<Process> processes_;
    std::vector<std::thread> threads_;
};

};