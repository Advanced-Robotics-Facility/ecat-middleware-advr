add_executable(advrf_zmq_server
    server.cpp
)

add_executable(advrf_zmq_client
    client.cpp
)

target_compile_features(advrf_zmq_server PRIVATE cxx_std_17)
target_compile_features(advrf_zmq_client PRIVATE cxx_std_17)

target_link_libraries(advrf_zmq_server PRIVATE cppzmq)
target_link_libraries(advrf_zmq_client PRIVATE cppzmq)
