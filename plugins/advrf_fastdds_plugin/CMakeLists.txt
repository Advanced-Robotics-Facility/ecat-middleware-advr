cmake_minimum_required(VERSION 3.16)

project(advrf_fastdds_plugin
    VERSION 0.1.0
    LANGUAGES C CXX
)

find_package(advrf_fastdds_lib REQUIRED)
find_package(advrf_interfaces_dds REQUIRED)
find_package(advrf_middleware_core REQUIRED)
find_package(advrf_interfaces_protobuf REQUIRED)
find_package(ecat_master_future REQUIRED)
find_package(yaml-cpp REQUIRED)

message(STATUS "Configuring FastDDS plugin")

add_library(ecat_plugin_fastdds
    src/main.cpp
)

target_compile_features(ecat_plugin_fastdds PUBLIC cxx_std_17)

target_include_directories(ecat_plugin_fastdds PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(ecat_plugin_fastdds
    PUBLIC
        advrf_interfaces_dds
        advrf_middleware_core
        advrf_interfaces_protobuf
        ecat_master_future
        fastdds
        fastcdr
)

option(BUILD_EXAMPLES "Build Fast DDS examples" OFF)

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
