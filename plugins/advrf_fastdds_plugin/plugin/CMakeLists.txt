add_executable(fastdds_plugin
    plugin.cpp
)

target_compile_features(fastdds_plugin
    PRIVATE
        cxx_std_17
)

target_link_libraries(fastdds_plugin
    PRIVATE
        advrf_fastdds_plugin
        advrf_middleware_core
        advrf_interfaces_dds::fastdds
        ecat_master_future
        yaml-cpp::yaml-cpp
)

target_compile_definitions(fastdds_plugin PRIVATE
        ROBOT_CONFIG_DIR="${advrf_middleware_core_CONFIG_DIR}/robot.yaml"
)

install(
    TARGETS fastdds_plugin
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)