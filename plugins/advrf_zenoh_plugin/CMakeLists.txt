cmake_minimum_required(VERSION 3.16)

project(advrf_zenoh_plugin
    VERSION 0.1.0
    LANGUAGES CXX
)

if(NOT BUILD_ZENOH)
    message(STATUS "Zenoh build disabled")
    return()
endif()

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

find_package(advrf_zenoh_lib REQUIRED)
find_package(advrf_middleware_core REQUIRED)
find_package(advrf_interfaces_protobuf REQUIRED)
find_package(yaml-cpp REQUIRED)
find_package(advrf_config CONFIG REQUIRED)

option(ZENOH_ROS2_SUPPORT
    "Build ROS2 CDR interoperability support"
    OFF
)

if(ZENOH_ROS2_SUPPORT)
    find_package(advrf_interfaces_dds REQUIRED)
    find_package(advrf_dds_common REQUIRED)
endif()

add_library(advrf_zenoh_plugin SHARED
    src/zenoh_adapter_publishers.cpp
    src/zenoh_adapter_subscribers.cpp
    #src/zenoh_adapter_services.cpp
)

if(ZENOH_ROS2_SUPPORT)
    target_sources(advrf_zenoh_plugin PRIVATE
        src/ros2cdr.cpp
    )

    target_link_libraries(advrf_zenoh_plugin PRIVATE
        advrf_interfaces_dds::cdr
        advrf_dds_common::cdr
    )

    target_compile_definitions(advrf_zenoh_plugin PUBLIC
        ZENOH_ROS2_SUPPORT=1
    )
endif()

add_library(advrf::zenoh_plugin ALIAS advrf_zenoh_plugin)

set_target_properties(advrf_zenoh_plugin PROPERTIES
    EXPORT_NAME zenoh_plugin
)

target_compile_features(advrf_zenoh_plugin PUBLIC cxx_std_17)

target_include_directories(advrf_zenoh_plugin
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(advrf_zenoh_plugin
    PUBLIC
        advrf_middleware_core
        advrf_interfaces_protobuf
        zenohcxx::zenohc
)

target_compile_definitions(advrf_zenoh_plugin PUBLIC
    $<$<CONFIG:Debug>:SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE>
    $<$<CONFIG:RelWithDebInfo>:SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_DEBUG>
    $<$<CONFIG:Release>:SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_INFO>
)

install(
    TARGETS advrf_zenoh_plugin
    EXPORT advrf_zenoh_pluginTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(
    EXPORT advrf_zenoh_pluginTargets
    FILE advrf_zenoh_pluginTargets.cmake
    NAMESPACE advrf::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

configure_package_config_file(
    cmake/advrf_zenoh_pluginConfig.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/advrf_zenoh_pluginConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/advrf_zenoh_pluginConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/advrf_zenoh_pluginConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/advrf_zenoh_pluginConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

option(BUILD_PLUGIN "Build the Zenoh plugin executable" ON)
if(BUILD_PLUGIN)
    add_subdirectory(plugin)
endif()

option(BUILD_EXAMPLES "Build Zenoh examples" OFF)
if(BUILD_EXAMPLES)
    add_subdirectory(test/examples)
endif()

option(BUILD_TESTING "Build Zenoh plugin tests" OFF)
if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(test/unit)
endif()
