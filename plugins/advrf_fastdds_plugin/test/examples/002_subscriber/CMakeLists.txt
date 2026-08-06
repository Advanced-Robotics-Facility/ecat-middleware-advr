add_executable(002_subscriber
    002_subscriber.cpp
)

target_compile_features(002_subscriber
    PRIVATE
        cxx_std_17
)

target_link_libraries(002_subscriber
    PRIVATE
        advrf_fastdds_plugin
        advrf_interfaces_dds::fastdds
        advrf_middleware_core
        yaml-cpp::yaml-cpp
)

target_compile_definitions(002_subscriber PRIVATE
    ROBOT_CONFIG_DIR="${advrf_middleware_core_CONFIG_DIR}/robot.yaml"
)

install(
    TARGETS 002_subscriber
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)