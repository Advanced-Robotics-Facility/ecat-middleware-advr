#pragma once
#include <atomic>
#include <array>
#include <cstdint>
#include <cstddef>
#include <cstring>

static constexpr const char* SHM_NAME = "/ecat_master";

template<typename T, size_t N>
struct SPSCQueue {
    static_assert((N & (N - 1)) == 0);
    static constexpr size_t MASK = N - 1;

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

        if (next == tail.load(std::memory_order_acquire)) 
            return false;

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

    size_t size() const {
        size_t h = head.load(std::memory_order_relaxed);
        size_t t = tail.load(std::memory_order_relaxed);
        return (h - t) & MASK;
    }
};

static constexpr size_t PROTO_MAX_BYTES = 512;
static constexpr size_t PROTO_FRAME_HEADER_BYTES = sizeof(uint32_t);

struct ProtoSlot {
    uint32_t size{0};
    uint8_t  data[PROTO_MAX_BYTES]{};
};

static constexpr size_t MAX_SLAVES_CAPACITY = 64;

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

struct SharedBridge {
    // Pub --> from EcatMaster to DDS
    SPSCQueue<ProtoSlot, 64> imu;
    SPSCQueue<ProtoSlot, 64> motor;
    SPSCQueue<ProtoSlot, 64> force_torque;
    SPSCQueue<ProtoSlot, 64> power_board;
    SPSCQueue<ProtoSlot, 64> pump;
    SPSCQueue<ProtoSlot, 64> valve;
    SPSCQueue<ProtoSlot, 64> gripper;

    alignas(64) std::atomic<uint32_t> topology_size {0};
    std::array<DiscoveredSlave, MAX_SLAVES_CAPACITY> topology {};
    
    alignas(64) std::atomic<bool> mw_ready{false};
    alignas(64) std::atomic<bool> rt_ready{false};
};

// Owns the intermediate serialization buffers 
// and provides drain/push logic shared by both processes.
struct ShmProtoHelper {
    ProtoSlot slot {};

    static bool frame_payload_size(const ProtoSlot& frame, uint32_t& payload_size) {
        if (frame.size < PROTO_FRAME_HEADER_BYTES || frame.size > PROTO_MAX_BYTES)
            return false;

        payload_size =
            static_cast<uint32_t>(frame.data[0]) |
            (static_cast<uint32_t>(frame.data[1]) << 8) |
            (static_cast<uint32_t>(frame.data[2]) << 16) |
            (static_cast<uint32_t>(frame.data[3]) << 24);

        return payload_size > 0 &&
               static_cast<size_t>(payload_size) + PROTO_FRAME_HEADER_BYTES == frame.size;
    }

    // Drain the queue and process every message (NRT usage)
    template<size_t N, typename Proto, typename Fn>
    void drain(SPSCQueue<ProtoSlot, N>& queue, Proto& msg, Fn&& on_msg) {
        while (queue.try_pop(slot)) {
            uint32_t payload_size = 0;
            if (!frame_payload_size(slot, payload_size))
                continue;
            msg.Clear();
            if (msg.ParseFromArray(slot.data + PROTO_FRAME_HEADER_BYTES,
                                   static_cast<int>(payload_size)))
                on_msg(msg);
        }
    }

    // Drain the queue but keep only latest (for RT usage)
    template<size_t N, typename Proto>
    bool parse_latest(SPSCQueue<ProtoSlot, N>& queue, Proto& msg) {
        bool parsed = false;
        Proto candidate;
        while (queue.try_pop(slot)) {
            uint32_t payload_size = 0;
            if (!frame_payload_size(slot, payload_size))
                continue;

            candidate.Clear();
            if (candidate.ParseFromArray(slot.data + PROTO_FRAME_HEADER_BYTES,
                                         static_cast<int>(payload_size))) {
                msg = candidate;
                parsed = true;
            }
        }
        return parsed;
    }

    // Drain the queue but keep only the latest framed protobuf bytes.
    template<size_t N>
    bool pop_latest_frame(SPSCQueue<ProtoSlot, N>& queue, ProtoSlot& frame) {
        bool popped = false;
        while (queue.try_pop(slot)) {
            uint32_t payload_size = 0;
            if (!frame_payload_size(slot, payload_size))
                continue;

            frame = slot;
            popped = true;
        }
        return popped;
    }

    template<size_t N, typename Proto>
    bool push(SPSCQueue<ProtoSlot, N>& queue, const Proto& msg) {
        // Ask Protobuf how many bytes the message will occupy once serialized
        const size_t payload_size = msg.ByteSizeLong();
        if (payload_size == 0 || payload_size + PROTO_FRAME_HEADER_BYTES > PROTO_MAX_BYTES)
            return false;

        const uint32_t payload_u32 = static_cast<uint32_t>(payload_size);
        slot.data[0] = static_cast<uint8_t>(payload_u32 & 0xFF);
        slot.data[1] = static_cast<uint8_t>((payload_u32 >> 8) & 0xFF);
        slot.data[2] = static_cast<uint8_t>((payload_u32 >> 16) & 0xFF);
        slot.data[3] = static_cast<uint8_t>((payload_u32 >> 24) & 0xFF);

        // Serialize the protobuf payload after the 4-byte little-endian length prefix.
        if (!msg.SerializeToArray(slot.data + PROTO_FRAME_HEADER_BYTES,
                                  static_cast<int>(payload_size)))
            return false;

        slot.size = static_cast<uint32_t>(payload_size + PROTO_FRAME_HEADER_BYTES);
        return queue.try_push(slot);
    }
};
