add_executable(001_sdo-request_001_parameters
    001_sdo-request_001_parameters.cpp
)

target_link_libraries(001_sdo-request_001_parameters
    PRIVATE
        ecat_plugin_cyclonedds
)

add_test(
    NAME 001_sdo-request_001_parameters
    COMMAND 001_sdo-request_001_parameters
)