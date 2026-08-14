add_executable(003_subscriber_publisher
    003_subscriber_publisher.cpp
)

target_compile_features(003_subscriber_publisher
    PRIVATE
        cxx_std_17
)

target_link_libraries(003_subscriber_publisher
    PRIVATE
        advrf_cyclonedds_plugin
        advrf_interfaces_dds::cyclonedds
        advrf_middleware_core
        advrf_config
)

install(
    TARGETS 003_subscriber_publisher
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)