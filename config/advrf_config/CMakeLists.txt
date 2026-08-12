cmake_minimum_required(VERSION 3.16)

project(advrf_config LANGUAGES NONE)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(
    PROJECT_CONFIG
    ""
    CACHE STRING
    "Project configuration to install"
)

if(PROJECT_CONFIG STREQUAL "")
    message(FATAL_ERROR
        "PROJECT_CONFIG is not set.\n"
        "Example: -DPROJECT_CONFIG=robot_a"
    )
endif()

set(
    SELECTED_CONFIG_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/configs/${PROJECT_CONFIG}"
)

if(NOT IS_DIRECTORY "${SELECTED_CONFIG_DIR}")
    message(FATAL_ERROR
        "Configuration '${PROJECT_CONFIG}' not found:\n"
        "${SELECTED_CONFIG_DIR}"
    )
endif()

message(STATUS "Selected project config: ${PROJECT_CONFIG}")

# Install selected config with a fixed destination
install(
    DIRECTORY "${SELECTED_CONFIG_DIR}/"
    DESTINATION "${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}"
)

# Generate CMake package config
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/advrf_configConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/advrf_configConfig.cmake"
    INSTALL_DESTINATION
        "${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/cmake"
)

# Install CMake package config
install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/advrf_configConfig.cmake"
    DESTINATION
        "${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/cmake"
)

# Install package metadata
install(
    FILES package.xml
    DESTINATION "${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}"
)