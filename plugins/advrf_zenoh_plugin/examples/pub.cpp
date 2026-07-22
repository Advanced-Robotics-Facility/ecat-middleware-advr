#include <chrono>
#include <cstdint>
#include <csignal>
#include <iostream>
#include <string>
#include <thread>

#include <zenoh.hxx>

namespace
{
volatile std::sig_atomic_t running = 1;

void stop(int)
{
    running = 0;
}
} // namespace

int main(int argc, char** argv)
{
    using namespace std::chrono_literals;

    const std::string key = argc > 1 ? argv[1] : "advrf/example/simple";
    const std::string payload = argc > 2 ? argv[2] : "Hello from advrf_zenoh_pub";

    std::signal(SIGINT, stop);
    std::signal(SIGTERM, stop);

    try
    {
        auto session = zenoh::Session::open(zenoh::Config::create_default());
        auto publisher = session.declare_publisher(zenoh::KeyExpr(key));

        std::cout << "Publishing on '" << key << "'. Press Ctrl-C to stop.\n";
        std::uint64_t sequence = 0;
        while (running)
        {
            const std::string message = "[" + std::to_string(sequence++) + "] " + payload;
            std::cout << "Publishing: " << message << '\n';
            publisher.put(message);
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
