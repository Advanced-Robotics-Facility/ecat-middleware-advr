add_executable(001_publisher
    001_publisher.cpp
)

target_compile_features(001_publisher
    PRIVATE
        cxx_std_17
)

target_link_libraries(001_publisher
    PRIVATE
        advrf_fastdds_plugin
        advrf_interfaces_dds::fastdds
        advrf_middleware_core
        ecat_master_future
        yaml-cpp::yaml-cpp
)

target_compile_definitions(001_publisher PRIVATE
    ROBOT_CONFIG_DIR="${advrf_middleware_core_CONFIG_DIR}/robot.yaml"
)

install(
    TARGETS 001_publisher
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)