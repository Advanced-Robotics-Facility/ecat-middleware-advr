add_executable(004_subscriber
    004_subscriber.cpp
)

target_compile_features(004_subscriber 
    PRIVATE 
        cxx_std_17
)

target_link_libraries(004_subscriber
    PRIVATE
        advrf_config
        advrf_interfaces_protobuf
        advrf_middleware_core
        zenohcxx::zenohc
)

install(
    TARGETS 004_subscriber
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
