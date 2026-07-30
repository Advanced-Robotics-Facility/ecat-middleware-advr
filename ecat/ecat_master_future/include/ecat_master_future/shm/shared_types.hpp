#pragma once
#include <atomic>
#include <array>
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <iostream>

#include "ecat_master_future/shm/config.hpp"

template<typename T, size_t N>
struct SPSCQueue {
    static_assert((N & (N - 1)) == 0);
    static constexpr size_t MASK = N - 1;
    static constexpr std::size_t capacity = N;

    // alignas(64): puts head and tail on two separate cache lines
    alignas(64) std::atomic<size_t> head{0};  // Producer only
    alignas(64) std::atomic<size_t> tail{0};  // Consumer only
    std::array<T, N> buf{};

    // Producer side (DDS process)
    bool try_push(const T& val) {
        // Read head index
        size_t h = head.load(std::memory_order_relaxed);   
        // Calculate the next head index
        size_t next = (h + 1) & MASK;

        if (next == tail.load(std::memory_order_acquire)) {
            return false;
        }

        // Write JointState data on buffer
        buf[h] = val;
        // Update head index
        head.store(next, std::memory_order_release);

        return true;
    }

    // Consumer side (RT process)
    bool try_pop(T& val) {
        // Read tail index
        size_t t = tail.load(std::memory_order_relaxed);

        if (t == head.load(std::memory_order_acquire)) 
            return false;
        
        // Read JointState data from buffer 
        val = buf[t];

        // Update tail index
        tail.store((t + 1) & MASK, std::memory_order_release);
        
        return true;
    }

    bool peek(size_t index, T& value) const
    {
        const size_t h = head.load(std::memory_order_acquire);
        const size_t t = tail.load(std::memory_order_acquire);

        const size_t count = (h - t) & MASK;

        if (index >= count)
            return false;

        value = buf[(t + index) & MASK];

        return true;
    }

    size_t size() const {
        size_t h = head.load(std::memory_order_relaxed);
        size_t t = tail.load(std::memory_order_relaxed);
        return (h - t) & MASK;
    }
};



struct ProtoSlot {
    uint32_t size{0};
    uint8_t  data[PROTO_MAX_BYTES]{};
};

enum class DeviceType : uint8_t {
    UNKNOWN      = 0,
    IMU          = 1,
    MOTOR        = 2,
    FORCE_TORQUE = 3,
    POWER_BOARD  = 4,
    PUMP         = 5,
    VALVE        = 6,
    GRIPPER      = 7
};

struct DiscoveredSlave {
    uint32_t board_id {0};
    DeviceType type {DeviceType::UNKNOWN};
    char name[32] {}; // 32 max safe length for SHM name
};
