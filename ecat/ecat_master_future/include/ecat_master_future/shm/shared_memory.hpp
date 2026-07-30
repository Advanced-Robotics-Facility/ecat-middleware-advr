#pragma once

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include <memory>
#include <string>
#include <cstring>
#include <csignal>

#include "ecat_master_future/shm/bridge_struct.hpp"

template<typename T_Bridge>
class SharedMemory
{
    static_assert(is_shared_memory_bridge_v<T_Bridge>,
                  "T must be a SharedMemoryBridge<...>");

public:
    static std::unique_ptr<SharedMemory> create(const std::string& name)
    {
        shm_unlink(name.c_str());
        auto shm = std::unique_ptr<SharedMemory>(
            new SharedMemory(name, O_CREAT | O_EXCL | O_RDWR));

        return shm->map_create() ? std::move(shm) : nullptr;
    }

    static std::unique_ptr<SharedMemory> open(const std::string& name)
    {
        auto shm = std::unique_ptr<SharedMemory>(
            new SharedMemory(name, O_RDWR));

        return shm->map_existing() ? std::move(shm) : nullptr;
    }

    static std::unique_ptr<SharedMemory> open_or_create(const std::string& name)
    {
        auto shm = std::unique_ptr<SharedMemory>(
            new SharedMemory(name, O_CREAT | O_RDWR));

        return shm->map_create() ? std::move(shm) : nullptr;
    }

    SharedMemory(const SharedMemory&) = delete;
    SharedMemory& operator=(const SharedMemory&) = delete;

    ~SharedMemory()
    {
        if (created_ && ptr_)
        {
            std::memset(ptr_, 0, sizeof(T_Bridge));
        }

        if (ptr_)
            munmap(ptr_, sizeof(T_Bridge));
    }

    bool created() const
    {
        return created_;
    }

    const std::string& name() const
    {
        return name_;
    }

    T_Bridge& object()
    {
        return *static_cast<T_Bridge*>(ptr_);
    }

    const T_Bridge& object() const
    {
        return *static_cast<const T_Bridge*>(ptr_);
    }

    T_Bridge& operator*()             { return object(); }
    const T_Bridge& operator*() const { return object(); }

    T_Bridge& bridge()             { return object(); }
    const T_Bridge& bridge() const { return object(); }

    bool owner_alive() const
    {
        pid_t pid = object().status.owner_pid.load(std::memory_order_acquire);
        return pid != 0 && (kill(pid, 0) == 0 || errno != ESRCH);
    }

    T_Bridge* operator->()            { return &object(); }
    const T_Bridge* operator->() const{ return &object(); }

    bool is_ok() const
    {
        return ptr_ != nullptr && owner_alive();
    }

    T_Bridge* get() const
    {
        return static_cast<T_Bridge*>(ptr_);
    }

private:
    SharedMemory(std::string name, int flags)
        : name_(std::move(name))
        , flags_(flags)
    {
    }

    bool map_existing()
    {
        return map(false);
    }

    bool map_create()
    {
        if (!map(true))
            return false;

        if (created_)
        {
            new (&object()) T_Bridge();
            object().status.owner_pid.store(
                getpid(),
                std::memory_order_release);
        }

        return true;
    }

    bool map(bool creating)
    {
        int fd = shm_open(name_.c_str(), flags_, 0666);
        if (fd < 0)
            return false;

        if (creating)
        {
            struct stat st{};
            if (fstat(fd, &st) != 0)
            {
                close(fd);
                return false;
            }

            created_ = (st.st_size == 0);
            if (created_)
            {
                if (ftruncate(fd, sizeof(T_Bridge)) != 0)
                {
                    close(fd);
                    return false;
                }
            }
        }

        ptr_ = mmap(nullptr,
                    sizeof(T_Bridge),
                    PROT_READ | PROT_WRITE,
                    MAP_SHARED,
                    fd,
                    0);

        close(fd);

        if (ptr_ == MAP_FAILED)
        {
            ptr_ = nullptr;
            return false;
        }

        return true;
    }

    std::string name_;
    int flags_;

    void* ptr_{nullptr};
    bool created_{false};
};