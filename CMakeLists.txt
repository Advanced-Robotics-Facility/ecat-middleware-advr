cmake_minimum_required(VERSION 3.16)

project(ecat_middleware LANGUAGES NONE)

include(ExternalProject)

get_filename_component(
    WORKSPACE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../.." ABSOLUTE
)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${WORKSPACE_ROOT}/install"
        CACHE PATH "Shared package installation prefix" FORCE)
endif()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release
        CACHE STRING "Build type" FORCE)
endif()

set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
    Debug Release RelWithDebInfo MinSizeRel)

option(BUILD_MIDDLEWARE_FASTDDS "Build the FastDDS backend" OFF)
option(BUILD_MIDDLEWARE_CYCLONEDDS "Build the CycloneDDS backend" ON)
option(BUILD_MIDDLEWARE_ZENOH "Build the Zenoh backend" OFF)
option(BUILD_MIDDLEWARE_ZMQ "Build the ZMQ backend" OFF)
option(BUILD_TEST_UNIT "Build tests" ON)
option(BUILD_TEST_EXAMPLES "Build examples" ON)

# Path containing robot configurations
set(CONFIG_DIR "${CMAKE_CURRENT_SOURCE_DIR}/config/advrf_config/configs")

# Find all entries in the configs directory
file(GLOB CONFIG_ENTRIES
    LIST_DIRECTORIES true
    "${CONFIG_DIR}/*"
)

set(AVAILABLE_CONFIGS "")

foreach(ENTRY ${CONFIG_ENTRIES})
    if(IS_DIRECTORY "${ENTRY}")
        get_filename_component(CONFIG_NAME "${ENTRY}" NAME)
        list(APPEND AVAILABLE_CONFIGS "${CONFIG_NAME}")
    endif()
endforeach()

set(SELECT_CONFIG "ecat-pegasus-config"
    CACHE STRING "Robot configuration"
)

set_property(CACHE SELECT_CONFIG PROPERTY STRINGS
    ${AVAILABLE_CONFIGS}
)

set(STAGING_PREFIX "${CMAKE_BINARY_DIR}/stage")

set(PACKAGE_PREFIX_PATH
    "${STAGING_PREFIX}"
    "${CMAKE_INSTALL_PREFIX}"
    ${CMAKE_PREFIX_PATH}
)
list(REMOVE_DUPLICATES PACKAGE_PREFIX_PATH)
string(REPLACE ";" "|" PACKAGE_PREFIX_PATH "${PACKAGE_PREFIX_PATH}")

function(add_package name relative_source)
    set(previous "${LAST_PACKAGE}")

    ExternalProject_Add(${name}
        SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/${relative_source}"
        BINARY_DIR "${CMAKE_BINARY_DIR}/packages/${name}"

        DOWNLOAD_COMMAND ""
        UPDATE_COMMAND ""

        LIST_SEPARATOR "|"

        CMAKE_ARGS
            "-DCMAKE_INSTALL_PREFIX:PATH=${STAGING_PREFIX}"
            "-DCMAKE_PREFIX_PATH:STRING=${PACKAGE_PREFIX_PATH}"
            "-DADVRF_RUNTIME_PREFIX:PATH=${CMAKE_INSTALL_PREFIX}"
            "-DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}"
            "-DSELECT_CONFIG:STRING=${SELECT_CONFIG}"
            "-DBUILD_MIDDLEWARE_FASTDDS:BOOL=${BUILD_MIDDLEWARE_FASTDDS}"
            "-DBUILD_MIDDLEWARE_CYCLONEDDS:BOOL=${BUILD_MIDDLEWARE_CYCLONEDDS}"
            "-DBUILD_MIDDLEWARE_ZENOH:BOOL=${BUILD_MIDDLEWARE_ZENOH}"
            "-DBUILD_TEST_UNIT:BOOL=${BUILD_TEST_UNIT}"
            "-DBUILD_TEST_EXAMPLES:BOOL=${BUILD_TEST_EXAMPLES}"

        BUILD_ALWAYS TRUE

        # Stage each package before configuring its consumers.
        INSTALL_COMMAND
            "${CMAKE_COMMAND}" --install <BINARY_DIR>
    )

    if(previous)
        add_dependencies(${name} ${previous})
    endif()

    # Install to the final prefix only when the outer install is requested.
    install(CODE "
        execute_process(
            COMMAND \"${CMAKE_COMMAND}\"
                --install \"${CMAKE_BINARY_DIR}/packages/${name}\"
                --prefix \"\${CMAKE_INSTALL_PREFIX}\"
            RESULT_VARIABLE install_result
        )

        if(NOT install_result EQUAL 0)
            message(FATAL_ERROR \"Failed to install ${name}\")
        endif()
    ")

    set(LAST_PACKAGE "${name}" PARENT_SCOPE)
endfunction()

## Common base

add_package(advrf_config config/advrf_config)
add_package(advrf_interfaces_protobuf interfaces/advrf_interfaces_protobuf)
add_package(advrf_middleware_core core/advrf_middleware_core)
add_package(shm_tools tools/shm_tools)
 
## Library compilations

if(BUILD_MIDDLEWARE_FASTDDS)
    add_package(advrf_fastdds_lib plugins/advrf_fastdds_lib)
endif()

if(BUILD_MIDDLEWARE_CYCLONEDDS)
    add_package(advrf_cyclonedds_lib plugins/advrf_cyclonedds_lib)
endif()

if(BUILD_MIDDLEWARE_ZENOH)
    add_package(advrf_zenoh_lib plugins/advrf_zenoh_lib)
endif()

if(BUILD_MIDDLEWARE_ZMQ)
    add_package(advrf_zmq_lib plugins/advrf_zmq_lib)
endif()

## DDS interfaces and common compilations

if(BUILD_MIDDLEWARE_FASTDDS 
    OR BUILD_MIDDLEWARE_CYCLONEDDS 
    OR BUILD_MIDDLEWARE_ZENOH)
    add_package(advrf_interfaces_dds interfaces/advrf_interfaces_dds)
    add_package(advrf_dds_common plugins/advrf_dds_common)
endif()

## Plugin compilations

if(BUILD_MIDDLEWARE_FASTDDS)
    add_package(advrf_fastdds_plugin plugins/advrf_fastdds_plugin)
endif()

if(BUILD_MIDDLEWARE_CYCLONEDDS)
    add_package(advrf_cyclonedds_plugin plugins/advrf_cyclonedds_plugin)
endif()

if(BUILD_MIDDLEWARE_ZENOH)
    add_package(advrf_zenoh_plugin plugins/advrf_zenoh_plugin)
endif()

if(BUILD_MIDDLEWARE_ZMQ)
    add_package(advrf_zmq_plugin plugins/advrf_zmq_plugin)
endif()