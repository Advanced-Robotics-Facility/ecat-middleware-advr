cmake_minimum_required(VERSION 3.16)

project(advrf_config LANGUAGES NONE)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(
    SELECT_CONFIG
    ""
    CACHE STRING
    "Project configuration to install"
)

if(SELECT_CONFIG STREQUAL "")
    message(FATAL_ERROR
        "SELECT_CONFIG is not set.\n"
        "Example: -DSELECT_CONFIG=robot_a"
    )
endif()

set(
    SELECTED_CONFIG_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/configs/${SELECT_CONFIG}"
)

if(NOT IS_DIRECTORY "${SELECTED_CONFIG_DIR}")
    message(FATAL_ERROR
        "Configuration '${SELECT_CONFIG}' not found:\n"
        "${SELECTED_CONFIG_DIR}"
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

# Remove previous config before installing the new one
install(CODE "
    message(STATUS \"Cleaning previous project config\")
    file(
        REMOVE_RECURSE
        \"\${CMAKE_INSTALL_PREFIX}/${CONFIG_INSTALL_DIR}\"
    )
")

# Install selected config
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