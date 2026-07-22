cmake_minimum_required(VERSION 3.16)

project(advrf_zenoh_plugin
    VERSION 0.1.0
    LANGUAGES C CXX
)

find_package(advrf_zenoh_lib REQUIRED)
find_package(advrf_middleware_core REQUIRED)
find_package(advrf_interfaces_protobuf REQUIRED)
find_package(ecat_master_future REQUIRED)

message(STATUS "Configuring zenoh plugin")

add_library(ecat_plugin_zenoh
    src/main.cpp
)

target_compile_features(ecat_plugin_zenoh PUBLIC cxx_std_17)

target_include_directories(ecat_plugin_zenoh PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(ecat_plugin_zenoh
    PUBLIC
        advrf_middleware_core
        advrf_interfaces_protobuf
        ecat_master_future
        zenohcxx::zenohc
)

option(BUILD_EXAMPLES "Build Zenoh examples" OFF)

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
