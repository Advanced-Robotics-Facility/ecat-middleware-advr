#pragma once
#include <ecat_master_future/shm_shared_types.hpp>

// TODO [hugo]: add a SHM interface to the ServiceServerCmd class to handle the shared memory communication with the RT side

enum SHMType {
    SHM_TYPE_DATA = 0,
    SHM_TYPE_REQUEST = 1
};

struct SHMStatus {
    alignas(64) std::atomic<bool> mw_ready{false};
    alignas(64) std::atomic<bool> rt_ready{false};
};

struct SHMTopology {
    alignas(64) std::atomic<uint32_t> topology_size {0};
    std::array<DiscoveredSlave, MAX_SLAVES_CAPACITY> topology {};
};

struct SHMRequestInfo
{
    ProtoSlot header;
    ProtoSlot status;
};

template<typename T_PAYLOAD>
struct SHMBaseData
{
    SHMStatus status;
    SHMType type{SHMType::SHM_TYPE_DATA};
    SHMTopology topology;
    T_PAYLOAD data;
};

struct SHMBaseSrv
{
    SHMStatus status;
    SHMTopology topology;
    SHMType type{SHMType::SHM_TYPE_REQUEST};
    SHMRequestInfo request_info;
    ProtoSlot request;
    ProtoSlot response;
};