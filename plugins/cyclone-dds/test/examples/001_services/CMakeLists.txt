add_executable(001_services_sdo-get-request-server
    001_services_sdo-get-request-server.cpp
)

target_link_libraries(001_services_sdo-get-request-server
    PRIVATE
        ecat_plugin_cyclonedds
)

add_executable(001_services_sdo-get-request-client
    001_services_sdo-get-request-client.cpp
)

target_link_libraries(001_services_sdo-get-request-client
    PRIVATE
        ecat_plugin_cyclonedds
)
