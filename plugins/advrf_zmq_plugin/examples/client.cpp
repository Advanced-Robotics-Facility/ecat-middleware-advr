#include <zmq.hpp>

#include <iostream>
#include <string_view>

int main(int argc, char** argv) {
    
    zmq::context_t ctx {1};

    zmq::socket_t sock {ctx, zmq::socket_type::req};
    sock.connect("tcp://localhost:5555");

    const std::string_view data {"Hello"};

    for (auto request_num = 0; request_num < 10; ++request_num) {
        std::cout << "Sending Hello " << request_num << "...\n";
        sock.send(zmq::buffer(data), zmq::send_flags::none);

        zmq::message_t reply {};
        sock.recv(reply, zmq::recv_flags::none);

        std::cout << "Received " << reply.to_string() << " (" << request_num << ")\n"; 
    }

    return 0;
}
