if(NOT EXISTS "${FASTDDSGEN_EXECUTABLE}")
    message(FATAL_ERROR
        "Fast DDS-Gen Java runtime does not exist: ${FASTDDSGEN_EXECUTABLE}"
    )
endif()

if(NOT EXISTS "${FASTDDSGEN_JAR}")
    message(FATAL_ERROR
        "Fast DDS-Gen jar does not exist: ${FASTDDSGEN_JAR}"
    )
endif()

foreach(IDL_FILE IN LISTS IDL_FILES)
    file(RELATIVE_PATH IDL_RELATIVE "${IDL_DIR}" "${IDL_FILE}")
    if(IDL_RELATIVE MATCHES "^\\.\\.")
        message(FATAL_ERROR
            "IDL file is outside the IDL root: ${IDL_FILE}"
        )
    endif()

    message(STATUS "FastDDSGen: ${IDL_RELATIVE}")

    execute_process(
        COMMAND "${FASTDDSGEN_EXECUTABLE}" -jar "${FASTDDSGEN_JAR}"
            -replace
            -cs
            -no-dependencies
            -I .
            -d "${OUTPUT_DIR}"
            "${IDL_RELATIVE}"
        WORKING_DIRECTORY "${IDL_DIR}"
        RESULT_VARIABLE ret
    )

    if(NOT ret EQUAL 0)
        message(FATAL_ERROR "fastddsgen failed for ${IDL_FILE}")
    endif()
endforeach()
