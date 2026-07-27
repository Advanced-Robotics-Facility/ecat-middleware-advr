add_executable(client
    003_client.cpp
)

add_executable(server
    003_server.cpp
)

target_compile_features(client PRIVATE cxx_std_17)
target_compile_features(server PRIVATE cxx_std_17)

target_link_libraries(client
    PRIVATE
        zenohcxx::zenohc
)

target_link_libraries(server
    PRIVATE
        zenohcxx::zenohc
)