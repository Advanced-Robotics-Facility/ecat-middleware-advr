#pragma once

#include <advrf_middleware_core/utils/log.hpp>

#include <ecat_master_future/shm/shared_memory.hpp>
#include <thread>

#define MS_WAIT_FOR_REMOTE_READY 100

template<class Derived, class Bridge>
class ShmMiddlewareBridgeConnection
{
protected:
    Bridge& bridge()
    {
        return shm_->bridge();
    }

    const Bridge& bridge() const
    {
        return shm_->bridge();
    }

public:
    bool connect(const std::string& name)
    {
        if (!wait_for_shared_memory(name))
            return false;

        if (!wait_for_remote())
            return false;

        derived().set_ready(true);
        return true;
    }

    void close()
    {
        stop_ = true;
        if (shm_)
            derived().set_ready(false);
        shm_.reset();
    }

    bool is_ok() const
    {
        return shm_ && shm_->is_ok();
    }

protected:
    bool wait_for_shared_memory(const std::string& name)
    {
        while (!stop_)
        {
            shm_ = SharedMemory<Bridge>::open(name);
            if (shm_)
                return true;

            LOG_WARN("Waiting for shared memory '{}'", name);
            std::this_thread::sleep_for(
                std::chrono::milliseconds(MS_WAIT_FOR_REMOTE_READY));
        }

        return false;
    }

    bool wait_for_remote()
    {
        if (derived().remote_ready())
            return true;

        LOG_ERROR("Remote not ready: {}", shm_->name());
        return false;
    }

private:
    Derived& derived()
    {
        return static_cast<Derived&>(*this);
    }

    std::unique_ptr<SharedMemory<Bridge>> shm_;
    bool stop_{false};
};