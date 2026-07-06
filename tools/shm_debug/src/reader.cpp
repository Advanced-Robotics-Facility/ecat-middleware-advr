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
    iit::advrf::MotorCmd cmd;

    while (true) {

        proto.parse_latest(bridge->cmd, cmd);

        std::cout << "\033[2J\033[H"; // Clear terminal (optional)

        std::cout << "Timestamp : "
                  << cmd.sec() << "."
                  << cmd.nanosec() << "\n\n";

        std::cout << "Number of motors: "
                  << cmd.motors_size() << "\n\n";

        for (int i = 0; i < cmd.motors_size(); ++i) {

            const auto& motor = cmd.motors(i);

            std::cout
                << "Motor[" << i << "]  "
                << "pos_ref=" << motor.pos_ref()
                << "  vel_ref=" << motor.vel_ref()
                << "  torque_ffwd=" << motor.torque_ffwd()
                << '\n';
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return 0;
}