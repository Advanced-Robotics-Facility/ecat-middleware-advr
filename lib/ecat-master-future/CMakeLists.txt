cmake_minimum_required(VERSION 3.16)

project(ecat-master-future
    VERSION 1.0.0
    LANGUAGES CXX
)

add_subdirectory(protobuf)

include(CMakePackageConfigHelpers)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(ecat-master-future SHARED
    src/shm_utils.cpp
    src/shm_shared_types.cpp
)

add_dependencies(ecat-master-future protobuf_msgs)

add_library(ecat-master-future::ecat-master-future ALIAS ecat-master-future)

target_include_directories(ecat-master-future
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/protobuf>
)

target_link_libraries(ecat-master-future PRIVATE protobuf_msgs)

## TEST EXE
add_executable(ecat_master_future_producer
    src/producer.cpp
)

target_link_libraries(ecat_master_future_producer
    PRIVATE
        ecat-master-future
        protobuf_msgs
)

install(TARGETS ecat_master_future_producer
    RUNTIME DESTINATION bin
)
## TEST EXE


set_target_properties(ecat-master-future PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
)

#
# Installation
#

install(TARGETS ecat-master-future
    EXPORT ecat-master-futureTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(DIRECTORY include/
    DESTINATION include
)

#
# Package configuration
#

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/ecat-master-futureConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/ecat-master-futureConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/ecat-master-futureConfig.cmake"
    INSTALL_DESTINATION lib/cmake/ecat-master-future
)

install(EXPORT ecat-master-futureTargets
    FILE ecat-master-futureTargets.cmake
    NAMESPACE ecat-master-future::
    DESTINATION lib/cmake/ecat-master-future
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/ecat-master-futureConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/ecat-master-futureConfigVersion.cmake"
    DESTINATION lib/cmake/ecat-master-future
)
