include(CTest)

function(plugin_test TEST_NAME TEST_SOURCE)
    add_executable(${TEST_NAME}
        ${TEST_SOURCE}
    )

    # Existing tests use assert() for setup and checks; keep them active in Release.
    if(MSVC)
        target_compile_options(${TEST_NAME} PRIVATE /UNDEBUG)
    else()
        target_compile_options(${TEST_NAME} PRIVATE -UNDEBUG)
    endif()

    target_link_libraries(${TEST_NAME}
        PRIVATE
            advrf_middleware_core
            advrf_zenoh_plugin
            ecat_master::ecat_core
            ecat_master::ecat_master_pb 
    )

    add_test(
        NAME ${TEST_NAME}
        COMMAND $<TARGET_FILE:${TEST_NAME}>
    )
endfunction()

plugin_test(
    001_proto_serialization
    001_proto_serialization/001_proto_serialization.cpp
)

if(ZENOH_ROS2_SUPPORT)
    plugin_test(
        002_motor_cdr_serialization
        002_motor_cdr_serialization/002_motor_cdr_serialization.cpp
    )

    target_link_libraries(002_motor_cdr_serialization PRIVATE fastcdr)

    plugin_test(
        003_imu_idl_cdr_serialization
        003_imu_idl_cdr_serialization/003_imu_idl_cdr_serialization.cpp
    )

    target_link_libraries(003_imu_idl_cdr_serialization PRIVATE
        fastcdr
        advrf_interfaces_dds::cdr
    )

    plugin_test(
        004_command_cdr_deserialization
        004_command_cdr_deserialization/004_command_cdr_deserialization.cpp
    )

    target_link_libraries(004_command_cdr_deserialization PRIVATE
        fastcdr
        advrf_interfaces_dds::cdr
    )
endif()

plugin_test(006_config 006_config/006_config.cpp)

target_link_libraries(006_config PRIVATE advrf_middleware_core_test_support)
