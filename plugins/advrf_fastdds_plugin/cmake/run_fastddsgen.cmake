foreach(IDL_FILE IN LISTS IDL_FILES)
    get_filename_component(MSG_NAME "${IDL_FILE}" NAME_WE)

    message(STATUS "FastDDSGen: ${MSG_NAME}")

    execute_process(
        COMMAND fastddsgen
            -replace
            -cs
            -I "${IDL_DIR}"
            -d "${OUTPUT_DIR}"
            "${IDL_FILE}"
        RESULT_VARIABLE ret
    )

    if(NOT ret EQUAL 0)
        message(FATAL_ERROR "fastddsgen failed for ${IDL_FILE}")
    endif()
endforeach()