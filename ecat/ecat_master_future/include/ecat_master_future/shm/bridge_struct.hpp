#pragma once

#include "ecat_master_future/shm/config.hpp"
#include "ecat_master_future/shm/shared_types.hpp"

struct SharedPubBridge {
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
    
    alignas(64) std::atomic<bool> mw_ready{false};
    alignas(64) std::atomic<bool> rt_ready{false};
};

struct SharedReplBridge {
    // Middleware -> EcatMaster (Repl_cmd)
    SPSCQueue<ProtoSlot, SHARED_REPL_BRIDGE_SIZE_QUEUE> request;
    // EcatMaster -> Middleware (Repl_info)
    SPSCQueue<ProtoSlot, SHARED_REPL_BRIDGE_SIZE_QUEUE> reply;

    alignas(64) std::atomic<bool> mw_ready{false};
    alignas(64) std::atomic<bool> rt_ready{false};
};

struct SharedSubBridge {
    SPSCQueue<ProtoSlot, SHARED_SUB_BRIDGE_SIZE_QUEUE> request;

    alignas(64) std::atomic<bool> mw_ready{false};
    alignas(64) std::atomic<bool> rt_ready{false};
};
