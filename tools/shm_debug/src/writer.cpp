#include <ecat_master_future/shm_shared_types.hpp>
#include <ecat_master_future/shm_utils.hpp>
#include <advrf_interfaces_protobuf/ecat_pdo.pb.h>

#include <chrono>
#include <iostream>
#include <thread>
#include <new>

int main()
{
    SharedMemoryOpenOrCreate shm(SHM_NAME, sizeof(SharedBridge));

    if (!shm.is_valid()) {
        std::cerr << "Failed to open/create shared memory.\n";
        return 1;
    }

    auto* bridge = static_cast<SharedBridge*>(shm.raw_ptr());

    if (shm.created()) {
        new (bridge) SharedBridge();
        std::cout << "Created shared memory.\n";
    }

    ShmProtoHelper proto;

    float pos = 0.0f;
    bool increasing = true;

    while (true) {

        if (increasing) {
            pos += 0.01f;
            if (pos >= 1.0f)
                increasing = false;
        } else {
            pos -= 0.01f;
            if (pos <= 0.0f)
                increasing = true;
        }

        iit::advrf::Ec_slave_pdo cmd;

        cmd.set_type(iit::advrf::Ec_slave_pdo::TX_CIA402);

        auto* tx = cmd.mutable_cia402_tx_pdo();
        tx->set_target_pos(pos);
        tx->set_target_vel(1.0f);
        tx->set_target_torque(0.0f);
        tx->set_target_current(0.0f);

        proto.push(bridge->motor, cmd);

        std::cout << "Wrote MotorCmd: pos_ref = " << pos << '\r' << std::flush;

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return 0;
}