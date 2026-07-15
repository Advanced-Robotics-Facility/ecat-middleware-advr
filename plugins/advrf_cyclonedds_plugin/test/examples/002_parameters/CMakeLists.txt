add_executable(002_parameters_parameter-client
    002_parameters_parameter-client.cpp
)

target_link_libraries(002_parameters_parameter-client
    PRIVATE
        advrf_cyclonedds_plugin
        advrf_interfaces_dds
        advrf_interfaces_protobuf
)

install(
    TARGETS 002_parameters_parameter-client
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

add_executable(002_parameters_parameter-server
    002_parameters_parameter-server.cpp
)

target_link_libraries(002_parameters_parameter-server
    PRIVATE
        advrf_cyclonedds_plugin
        advrf_interfaces_dds
        advrf_interfaces_protobuf
)

install(
    TARGETS 002_parameters_parameter-server
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)