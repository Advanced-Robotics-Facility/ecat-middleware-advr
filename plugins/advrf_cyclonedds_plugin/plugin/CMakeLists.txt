add_executable(cyclonedds_plugin
    plugin.cpp
)

target_compile_features(cyclonedds_plugin
    PRIVATE
        cxx_std_17
)

target_link_libraries(cyclonedds_plugin
    PRIVATE
        advrf_cyclonedds_plugin
        advrf_middleware_core
        advrf_interfaces_dds::cyclonedds
        yaml-cpp::yaml-cpp
)

target_compile_definitions(cyclonedds_plugin PRIVATE
        ROBOT_CONFIG_DIR="${advrf_middleware_core_CONFIG_DIR}/robot.yaml"
)

install(
    TARGETS cyclonedds_plugin
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)