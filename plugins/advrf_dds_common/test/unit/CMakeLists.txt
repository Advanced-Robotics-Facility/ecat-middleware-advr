function(new_test TEST_NAME TEST_SOURCE)
    add_executable(${TEST_NAME} ${TEST_SOURCE})

    if(TARGET advrf_dds_common_cyclonedds)
        target_link_libraries(${TEST_NAME}
            PRIVATE
                advrf_dds_common_cyclonedds
        )
    elseif(TARGET advrf_dds_common_fastdds)
        target_link_libraries(${TEST_NAME}
            PRIVATE
                advrf_dds_common_fastdds
        )
    elseif(TARGET advrf_dds_common_cdr)
        target_link_libraries(${TEST_NAME}
            PRIVATE
                advrf_dds_common_cdr
        )
    else()
        message(FATAL_ERROR "No DDS common backend target available")
    endif()

    add_test(
        NAME ${TEST_NAME}
        COMMAND $<TARGET_FILE:${TEST_NAME}>
    )
endfunction()

new_test(
    001_converter
    001_converter/001_converter.cpp
)

new_test(
    002_motor_xt_tx_conversion
    002_motor_xt_tx_conversion/002_motor_xt_tx_conversion.cpp
)

new_test(
    003_motor_tx_conversion
    003_motor_tx_conversion/003_motor_tx_conversion.cpp
)