#include <ecat_master_future/shm_shared_types.hpp>
#include <ecat_master_future/shm_utils.hpp>
#include <advrf_interfaces_protobuf/ecat_pdo.pb.h>
#include <advrf_interfaces_protobuf/repl_cmd.pb.h>

#include <chrono>
#include <iostream>
#include <thread>

template <typename Bridge, typename Prototype>
class ReadBridge
{
    public:
        ReadBridge(const std::string& shm_name)
            : shm_(shm_name.c_str(), sizeof(Bridge))
        {
            shm_name_ = shm_name;
            if (!shm_.is_valid()) {
                std::cerr << "Failed to open shared memory segment." << '\n';
                std::exit(1);
            }   
            bridge_ = static_cast<Bridge*>(shm_.raw_ptr());
        }

        Prototype read_latest() {
            Prototype proto;
            proto_helper_.parse_latest(bridge_->motors_pdo, proto);
            return proto;
        }

        void start() {
            while (true) {
                std::cout << "SHM NAME: " << shm_name_ << std::endl;
                read_latest().PrintDebugString();
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }

    protected:
        ShmProtoHelper proto_helper_;
        SharedMemoryOpenOrCreate shm_;
        Bridge* bridge_;
        std::string shm_name_;
};

int main(int argc, char** argv)
{
    ReadBridge<SharedSubBridge, iit::advrf::Motors_PDO_cmd> (SHM_SUB_NAME).start();
    return 0;
}