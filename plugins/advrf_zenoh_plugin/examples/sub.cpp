#include <chrono>
#include <cstdint>
#include <csignal>
#include <cstdlib>
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

std::string decode_cdr_string(const zenoh::Bytes& payload)
{
    const auto bytes = payload.as_vector();
    if (bytes.size() < 9 || bytes[0] != 0x00 || bytes[1] != 0x01)
    {
        return payload.as_string();
    }

    const std::uint32_t length =
        static_cast<std::uint32_t>(bytes[4]) |
        (static_cast<std::uint32_t>(bytes[5]) << 8) |
        (static_cast<std::uint32_t>(bytes[6]) << 16) |
        (static_cast<std::uint32_t>(bytes[7]) << 24);

    if (length == 0 || 8U + length > bytes.size() || bytes[8U + length - 1U] != 0)
    {
        return payload.as_string();
    }

    return std::string(
        reinterpret_cast<const char*>(bytes.data() + 8),
        length - 1U);
}
} // namespace

int main(int argc, char** argv)
{
    using namespace std::chrono_literals;

    const std::string key = argc > 1 ? argv[1] : "advrf/example/simple";

    std::signal(SIGINT, stop);
    std::signal(SIGTERM, stop);

    try
    {
        auto config = zenoh::Config::create_default();
        auto session = zenoh::Session::open(std::move(config));
        auto subscriber = session.declare_subscriber(
            zenoh::KeyExpr(key),
            [](const zenoh::Sample& sample)
            {
                std::cout << "Received '"
                          << sample.get_keyexpr().as_string_view()
                          << "': " << decode_cdr_string(sample.get_payload()) << '\n';
            },
            zenoh::closures::none);

        std::cout << "Subscribed to '" << key << "'. Press Ctrl-C to stop.\n";
        while (running)
        {
            std::this_thread::sleep_for(200ms);
        }
    }
    catch (const zenoh::ZException& error)
    {
        std::cerr << "Zenoh subscriber error: " << error.what() << '\n';
        return 1;
    }

    return 0;
}
