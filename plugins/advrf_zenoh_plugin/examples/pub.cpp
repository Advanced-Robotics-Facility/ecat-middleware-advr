#include <chrono>
#include <cstdint>
#include <csignal>
#include <cstdlib>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include <zenoh.hxx>

namespace
{
volatile std::sig_atomic_t running = 1;

void stop(int)
{
    running = 0;
}

// CDR-encode a single std_msgs/String field:
// 4-byte encapsulation header + uint32 length (incl. null term) + bytes + null term, padded to 4.
std::vector<uint8_t> cdr_string(const std::string& s)
{
    std::vector<uint8_t> b = {0x00, 0x01, 0x00, 0x00};
    uint32_t len = static_cast<uint32_t>(s.size()) + 1;
    for (int i = 0; i < 4; ++i)
        b.push_back((len >> (8 * i)) & 0xFF);
    b.insert(b.end(), s.begin(), s.end());
    b.push_back(0x00);
    while (b.size() % 4)
        b.push_back(0x00);
    return b;
}
}

int main(int argc, char** argv)
{
    using namespace std::chrono_literals;

    const std::string key = argc > 1 ? argv[1] : "advrf/example/simple";
    const std::string payload = argc > 2 ? argv[2] : "Hello from advrf_zenoh_pub";

    std::signal(SIGINT, stop);
    std::signal(SIGTERM, stop);

    try
    {
        auto session = zenoh::Session::open(std::move(zenoh::Config::create_default()));
        auto publisher = session.declare_publisher(zenoh::KeyExpr(key));

        std::cout << "Publishing on '" << key << "'. Press Ctrl-C to stop.\n";

        std::uint64_t sequence = 0;
        while (running) {
            const std::string message = "[" + std::to_string(sequence++) + "] " + payload;
            std::cout << "Publishing: " << message << '\n';
            publisher.put(zenoh::Bytes(cdr_string(message)));
            std::this_thread::sleep_for(1s);
        }
    }
    catch (const zenoh::ZException& error)
    {
        std::cerr << "Zenoh publisher error: " << error.what() << '\n';
        return 1;
    }

    return 0;
}
