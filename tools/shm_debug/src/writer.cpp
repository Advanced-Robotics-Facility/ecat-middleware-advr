#include "shm_shared_types.hpp"
#include "shm_utils.hpp"
#include "motor.pb.h"

#include <chrono>
#include <iostream>
#include <thread>

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

        iit::advrf::MotorCmd cmd;

        cmd.mutable_motors()->Reserve(12);

        for (int i = 0; i < 12; ++i) {
            auto* motor = cmd.add_motors();

            motor->set_pos_ref(pos);
            motor->set_vel_ref(0.0f);
            motor->set_torque_ffwd(0.0f);
        }

        cmd.set_sec(0);
        cmd.set_nanosec(0);

        proto.push(bridge->cmd, cmd);

        std::cout << "Wrote MotorCmd: pos_ref = " << pos << '\r' << std::flush;

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return 0;
}