cmake_minimum_required(VERSION 3.16)

project(advrf_config LANGUAGES NONE)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(
    CONFIG_ROOT
    "${CMAKE_CURRENT_SOURCE_DIR}/configs"
)

# Find available configurations
file(
    GLOB CONFIG_ENTRIES
    RELATIVE "${CONFIG_ROOT}"
    "${CONFIG_ROOT}/*"
)

set(AVAILABLE_CONFIGS "")

foreach(CONFIG_NAME IN LISTS CONFIG_ENTRIES)
    if(IS_DIRECTORY "${CONFIG_ROOT}/${CONFIG_NAME}")
        list(APPEND AVAILABLE_CONFIGS "${CONFIG_NAME}")
    endif()
endforeach()

list(SORT AVAILABLE_CONFIGS)
string(JOIN ", " AVAILABLE_CONFIGS_STR ${AVAILABLE_CONFIGS})
if(AVAILABLE_CONFIGS)
    list(GET AVAILABLE_CONFIGS 0 FIRST_CONFIG)
else()
    set(FIRST_CONFIG "<config_name>")
endif()

set(
    SELECT_CONFIG
    ""
    CACHE STRING
    "Project configuration to install"
)

# Require a configuration
if(SELECT_CONFIG STREQUAL "")
    message(FATAL_ERROR
        "SELECT_CONFIG is not set.\n"
        "Available configs: ${AVAILABLE_CONFIGS_STR}\n"
        "Example: -DSELECT_CONFIG=${FIRST_CONFIG}"
    )
endif()

set(
    SELECTED_CONFIG_DIR
    "${CONFIG_ROOT}/${SELECT_CONFIG}"
)

# Check selected configuration
if(NOT IS_DIRECTORY "${SELECTED_CONFIG_DIR}")
    message(FATAL_ERROR
        "Configuration '${SELECT_CONFIG}' not found.\n"
        "Available configs: ${AVAILABLE_CONFIGS_STR}"
    )
endif()

message(STATUS "Selected project config: ${SELECT_CONFIG}")

# Fixed install locations
set(
    CONFIG_INSTALL_DIR
    "${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/config"
)

set(
    CMAKE_CONFIG_INSTALL_DIR
    "${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/cmake"
)

# Remove previous configuration
install(CODE "
    message(STATUS \"Cleaning previous project config\")
    file(
        REMOVE_RECURSE
        \"\${CMAKE_INSTALL_PREFIX}/${CONFIG_INSTALL_DIR}\"
    )
")

# Install selected configuration
install(
    DIRECTORY "${SELECTED_CONFIG_DIR}/"
    DESTINATION "${CONFIG_INSTALL_DIR}"
)

# Generate CMake package config
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/advrf_configConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/advrf_configConfig.cmake"
    INSTALL_DESTINATION "${CMAKE_CONFIG_INSTALL_DIR}"
    PATH_VARS CONFIG_INSTALL_DIR
)

# Install CMake package config
install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/advrf_configConfig.cmake"
    DESTINATION
        "${CMAKE_CONFIG_INSTALL_DIR}"
)

# Install package metadata
install(
    FILES package.xml
    DESTINATION "${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}"
)