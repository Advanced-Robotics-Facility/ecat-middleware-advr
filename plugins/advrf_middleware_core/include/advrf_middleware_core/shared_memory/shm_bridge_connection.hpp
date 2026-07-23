#pragma once

#include <chrono>
#include <memory>
#include <thread>

#include <ecat_master_future/shm_utils.hpp>
#include "advrf_middleware_core/utils/log.hpp"

template<class Derived, class Bridge>
class ShmBridgeConnection
{
public:
    ~ShmBridgeConnection()
    {
        close();
    }

    bool connect(const std::string& shm_name)
    {
        stop_ = false;
        while (!stop_)
        {
            shm_ = std::make_unique<SharedMemoryClient>(
                shm_name.c_str(),
                sizeof(Bridge));

            if (shm_->is_valid())
                break;

            LOG_DEBUG("Waiting shared memory {}", shm_name);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        if (stop_ || !shm_ || !shm_->is_valid())
            return false;

        bridge_ = shm_->template get<Bridge>();

        if (!bridge_)
            return false;

        while (!stop_ && !derived().peer_ready())
            std::this_thread::sleep_for(std::chrono::milliseconds(10));

        if (stop_)
            return false;

        derived().set_local_ready(true);

        return true;
    }

    void close()
    {
        stop_ = true;

        if (bridge_)
            derived().set_local_ready(false);

        bridge_ = nullptr;
        shm_.reset();
    }

    bool is_connected() const
    {
        return bridge_ != nullptr;
    }

    bool is_ok() const
    {
        return bridge_ && derived().peer_ready();
    }

    Bridge& bridge()
    {
        return *bridge_;
    }

    const Bridge& bridge() const
    {
        return *bridge_;
    }

protected:
    Derived& derived()
    {
        return static_cast<Derived&>(*this);
    }

    const Derived& derived() const
    {
        return static_cast<const Derived&>(*this);
    }

    Bridge* bridge_ = nullptr;

private:
    std::unique_ptr<SharedMemoryClient> shm_;
    bool stop_ = false;
};