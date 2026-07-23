add_executable(003_publisher_from_shm
    003_publisher_from_shm.cpp
)

target_compile_features(003_publisher_from_shm
    PRIVATE
        cxx_std_17
)

target_link_libraries(003_publisher_from_shm
    PRIVATE
        advrf_cyclonedds_plugin
        yaml-cpp::yaml-cpp
)

target_compile_definitions(003_publisher_from_shm PRIVATE
    ROBOT_CONFIG_DIR="${advrf_middleware_core_CONFIG_DIR}/robot.yaml"
)

install(
    TARGETS 003_publisher_from_shm
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
