cmake_minimum_required(VERSION 3.16)

project(advrf_cyclonedds_plugin
    VERSION 0.1.0
    LANGUAGES CXX
)

add_compile_options(
    $<$<CONFIG:Release>:-O3>
    $<$<CONFIG:RelWithDebInfo>:-O3>
)

find_package(advrf_cyclonedds_lib REQUIRED)
find_package(advrf_interfaces_dds REQUIRED)
find_package(advrf_middleware_core REQUIRED)
find_package(advrf_interfaces_protobuf REQUIRED)
find_package(ecat_master_future REQUIRED)
find_package(yaml-cpp REQUIRED)

message(STATUS "ecat_master_future_DIR = ${ecat_master_future_DIR}")
message(STATUS "ecat_master_future_CONFIG_DIR = ${ecat_master_future_CONFIG_DIR}")

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

add_library(advrf_cyclonedds_plugin
    src/parameter_make_get.cpp
    src/service_server.cpp
    src/parameter_client.cpp
    src/parameter_server.cpp
    src/dds_adapter_service.cpp
)

target_compile_features(advrf_cyclonedds_plugin
    PUBLIC
        cxx_std_17
)

target_include_directories(advrf_cyclonedds_plugin
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(advrf_cyclonedds_plugin
    PUBLIC
        advrf_middleware_core
        advrf_interfaces_dds
        advrf_interfaces_protobuf
        ecat_master_future
        CycloneDDS-CXX::ddscxx
        spdlog::spdlog
)

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

target_compile_definitions(advrf_cyclonedds_plugin PUBLIC
    $<$<CONFIG:Debug>:SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE>
    $<$<CONFIG:RelWithDebInfo>:SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_DEBUG>
    $<$<CONFIG:Release>:SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_INFO>
)

install(
    TARGETS advrf_cyclonedds_plugin
    EXPORT advrf_cyclonedds_pluginTargets
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
    EXPORT advrf_cyclonedds_pluginTargets
    FILE advrf_cyclonedds_pluginTargets.cmake
    NAMESPACE advrf::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/advrf_cyclonedds_pluginConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/advrf_cyclonedds_pluginConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/advrf_cyclonedds_pluginConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/advrf_cyclonedds_pluginConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/advrf_cyclonedds_pluginConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

option(BUILD_TEST_EXAMPLES "Build testing examples" ON)
option(BUILD_TESTING "Build unit tests" ON)

if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(test/unit)    
endif()

if(BUILD_TEST_EXAMPLES)
    add_subdirectory(test/examples)    
endif()


## Exec Plugin


add_executable(cyclonedds_plugin
    src/plugin.cpp
)

target_compile_features(cyclonedds_plugin
    PRIVATE
        cxx_std_17
)

target_link_libraries(cyclonedds_plugin
    PRIVATE
        advrf_cyclonedds_plugin
        yaml-cpp::yaml-cpp
)


target_compile_definitions(cyclonedds_plugin PRIVATE
    ROBOT_CONFIG_DIR="${ecat_master_future_CONFIG_DIR}/robot.yaml"
)

install(
    TARGETS cyclonedds_plugin
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)