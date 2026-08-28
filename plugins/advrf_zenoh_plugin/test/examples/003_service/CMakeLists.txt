add_executable(003_client
    003_client.cpp
)

target_compile_features(003_client 
    PRIVATE 
        cxx_std_17
)

target_link_libraries(003_client
    PRIVATE
        zenohcxx::zenohc
)

install(
    TARGETS 003_client
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# ---

add_executable(003_server
    003_server.cpp
)

target_compile_features(003_server 
    PRIVATE 
        cxx_std_17
)

target_link_libraries(003_server
    PRIVATE
        zenohcxx::zenohc
)

install(
    TARGETS 003_server
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)