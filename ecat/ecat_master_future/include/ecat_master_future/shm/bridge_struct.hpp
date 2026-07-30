#pragma once

#include "ecat_master_future/shm/config.hpp"
#include "ecat_master_future/shm/shared_types.hpp"

struct alignas(64) SHMStatus
{
    std::atomic<bool> mw_ready{false};
    std::atomic<bool> rt_ready{false};
    std::atomic<uint32_t> owner_pid{0};
};


struct SharedPubPayload {
    // Pub --> from EcatMaster to DDS
    SPSCQueue<ProtoSlot, SHARED_PUB_BRIDGE_SIZE_QUEUE> imu;
    SPSCQueue<ProtoSlot, SHARED_PUB_BRIDGE_SIZE_QUEUE> motor;
    SPSCQueue<ProtoSlot, SHARED_PUB_BRIDGE_SIZE_QUEUE> force_torque;
    SPSCQueue<ProtoSlot, SHARED_PUB_BRIDGE_SIZE_QUEUE> power_board;
    SPSCQueue<ProtoSlot, SHARED_PUB_BRIDGE_SIZE_QUEUE> pump;
    SPSCQueue<ProtoSlot, SHARED_PUB_BRIDGE_SIZE_QUEUE> valve;
    SPSCQueue<ProtoSlot, SHARED_PUB_BRIDGE_SIZE_QUEUE> gripper;

    alignas(64) std::atomic<uint32_t> topology_size {0};
    std::array<DiscoveredSlave, MAX_SLAVES_CAPACITY> topology {};
};

struct SharedReplPayload {
    // Middleware -> EcatMaster (Repl_cmd)
    SPSCQueue<ProtoSlot, SHARED_REPL_BRIDGE_SIZE_QUEUE> request;
    // EcatMaster -> Middleware (Repl_info)
    SPSCQueue<ProtoSlot, SHARED_REPL_BRIDGE_SIZE_QUEUE> reply;
};

struct SharedSubPayload {
    SPSCQueue<ProtoSlot, SHARED_SUB_BRIDGE_SIZE_QUEUE> request;
};

template<typename T>
struct SharedMemoryBridge
{
    SHMStatus status;
    T payload;
};

using SharedPubBridge  = SharedMemoryBridge<SharedPubPayload>;
using SharedReplBridge = SharedMemoryBridge<SharedReplPayload>;
using SharedSubBridge  = SharedMemoryBridge<SharedSubPayload>;

template<typename>
struct is_shared_memory_bridge : std::false_type {};

template<typename Payload>
struct is_shared_memory_bridge<SharedMemoryBridge<Payload>>
    : std::true_type {};

template<typename T>
inline constexpr bool is_shared_memory_bridge_v =
    is_shared_memory_bridge<T>::value;