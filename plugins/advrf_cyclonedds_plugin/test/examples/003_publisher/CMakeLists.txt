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
        advrf_interfaces_dds::cyclonedds
        advrf_middleware_core
        advrf_config
        yaml-cpp::yaml-cpp
)

install(
    TARGETS 003_publisher_from_shm
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
