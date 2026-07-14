message(STATUS "Configuring FastDDS plugin")

find_package(fastcdr 2 REQUIRED)
find_package(fastdds 3 REQUIRED)

if(NOT DDS_IDL_FILES)
    message(FATAL_ERROR "DDS_IDL_FILES not defined. interfaces/dds not loaded.")
endif()

set(FAST_GEN_DIR "${CMAKE_CURRENT_BINARY_DIR}/fastdds_generated")

set(ALL_GEN_SOURCES "")
set(ALL_GEN_OUTPUTS "")

foreach(IDL_FILE ${DDS_IDL_FILES})
    get_filename_component(MSG_NAME ${IDL_FILE} NAME_WE)

    list(APPEND ALL_GEN_SOURCES
        "${FAST_GEN_DIR}/${MSG_NAME}PubSubTypes.cxx"
        "${FAST_GEN_DIR}/${MSG_NAME}TypeObjectSupport.cxx"
    )

    list(APPEND ALL_GEN_OUTPUTS
        "${FAST_GEN_DIR}/${MSG_NAME}PubSubTypes.cxx"
        "${FAST_GEN_DIR}/${MSG_NAME}TypeObjectSupport.cxx"
        "${FAST_GEN_DIR}/${MSG_NAME}.hpp"
        "${FAST_GEN_DIR}/${MSG_NAME}PubSubTypes.hpp"
        "${FAST_GEN_DIR}/${MSG_NAME}TypeObjectSupport.hpp"
    )
endforeach()

add_custom_command(
    OUTPUT ${ALL_GEN_OUTPUTS}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${FAST_GEN_DIR}
    COMMAND ${CMAKE_COMMAND}
        -D "IDL_FILES=${DDS_IDL_FILES}"
        -D "IDL_DIR=${DDS_IDL_DIR}"
        -D "OUTPUT_DIR=${FAST_GEN_DIR}"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/run_fastddsgen.cmake"
    DEPENDS ${DDS_IDL_FILES}
    COMMENT "FastDDSGen: generating IDL sources"
    VERBATIM
)

add_custom_target(fastdds_generate DEPENDS ${ALL_GEN_OUTPUTS})

add_library(ecat_plugin_fastdds
    src/main.cpp
    ${ALL_GEN_SOURCES}
)

add_dependencies(ecat_plugin_fastdds fastdds_generate)

target_include_directories(ecat_plugin_fastdds PUBLIC
    ${FAST_GEN_DIR}
    ${DDS_IDL_DIR}
)

target_link_libraries(ecat_plugin_fastdds PUBLIC
    fastdds
    fastcdr
)