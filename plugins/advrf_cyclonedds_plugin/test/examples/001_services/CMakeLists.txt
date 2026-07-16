add_executable(001_services_sdo-get-request-server
    001_services_sdo-get-request-server.cpp
)

target_link_libraries(001_services_sdo-get-request-server
    PRIVATE
        advrf_cyclonedds_plugin
)

install(
    TARGETS 001_services_sdo-get-request-server
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

add_executable(001_services_sdo-get-request-client
    001_services_sdo-get-request-client.cpp
)

target_link_libraries(001_services_sdo-get-request-client
    PRIVATE
        advrf_cyclonedds_plugin
)

install(
    TARGETS 001_services_sdo-get-request-client
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)


add_executable(001_services_cmd_server
    001_services_cmd_server.cpp
)

target_link_libraries(001_services_cmd_server
    PRIVATE
        advrf_cyclonedds_plugin
)

install(
    TARGETS 001_services_cmd_server
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)