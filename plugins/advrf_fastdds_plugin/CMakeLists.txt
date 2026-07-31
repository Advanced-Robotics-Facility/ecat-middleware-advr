cmake_minimum_required(VERSION 3.16)

project(advrf_fastdds_plugin
    VERSION 0.1.0
    LANGUAGES C CXX
)

find_package(advrf_fastdds_lib REQUIRED)
find_package(advrf_dds_common REQUIRED)
find_package(advrf_interfaces_dds REQUIRED)
find_package(advrf_middleware_core REQUIRED)
find_package(advrf_interfaces_protobuf REQUIRED)
find_package(ecat_master_future REQUIRED)
find_package(yaml-cpp REQUIRED)

message(STATUS "ecat_master_future_DIR = ${ecat_master_future_DIR}")
message(STATUS "ecat_master_future_CONFIG_DIR = ${ecat_master_future_CONFIG_DIR}")

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

add_library(advrf_fastdds_plugin SHARED
    src/dds_adapter_publishers.cpp
    src/dds_adapter_subscribers.cpp
    src/dds_adapter_bridges.cpp
    src/dds_adapter_service.cpp
)

target_compile_features(advrf_fastdds_plugin 
    PUBLIC 
        cxx_std_17
)

target_include_directories(advrf_fastdds_plugin 
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(advrf_fastdds_plugin
    PUBLIC
        advrf_interfaces_dds::fastdds
        advrf_dds_common::fastdds
        advrf_middleware_core
        advrf_interfaces_protobuf
        fastdds
        fastcdr
    PRIVATE
        ecat_master_future
        spdlog::spdlog
)

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

target_compile_definitions(advrf_fastdds_plugin PUBLIC
    $<$<CONFIG:Debug>:SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE>
    $<$<CONFIG:RelWithDebInfo>:SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_DEBUG>
    $<$<CONFIG:Release>:SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_INFO>
)

install(
    TARGETS advrf_fastdds_plugin
    EXPORT advrf_fastdds_pluginTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
    DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
    EXPORT advrf_fastdds_pluginTargets
    FILE advrf_fastdds_pluginTargets.cmake
    NAMESPACE advrf::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/advrf_fastdds_pluginConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/advrf_fastdds_pluginConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/advrf_fastdds_pluginConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/advrf_fastdds_pluginConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/advrf_fastdds_pluginConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

option(BUILD_TEST_EXAMPLES "Build testing examples" OFF)
option(BUILD_TESTING "Build unit tests" OFF)

if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(test/unit)    
endif()

if(BUILD_TEST_EXAMPLES)
    add_subdirectory(test/examples)    
endif()

option(BUILD_PLUGIN "Build plugin" ON)
if(BUILD_PLUGIN)
    add_subdirectory(plugin/)    
endif()
