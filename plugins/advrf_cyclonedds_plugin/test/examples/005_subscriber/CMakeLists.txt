add_executable(005_subscriber_publisher
    005_subscriber_publisher.cpp
)

target_compile_features(005_subscriber_publisher
    PRIVATE
        cxx_std_17
)

target_link_libraries(005_subscriber_publisher
    PRIVATE
        advrf_cyclonedds_plugin
        advrf_interfaces_dds::cyclonedds
        advrf_middleware_core
)

install(
    TARGETS 005_subscriber_publisher
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)