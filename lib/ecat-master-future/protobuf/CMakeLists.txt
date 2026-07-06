cmake_minimum_required(VERSION 3.16)

project(protobuf_msgs
    VERSION 1.0.0
    LANGUAGES CXX
)

include(CMakePackageConfigHelpers)

find_package(Protobuf REQUIRED)

set(PROTO_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/motor.proto
    ${CMAKE_CURRENT_SOURCE_DIR}/imu.proto
)

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILES})

add_library(protobuf_msgs STATIC
    ${PROTO_SRCS}
    ${PROTO_HDRS}
)

add_library(protobuf_msgs::protobuf_msgs ALIAS protobuf_msgs)

target_include_directories(protobuf_msgs
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
        $<INSTALL_INTERFACE:include/protobuf_msgs>
)

target_link_libraries(protobuf_msgs
    PUBLIC
        protobuf::libprotobuf
)

#
# Installation
#

install(TARGETS protobuf_msgs
    EXPORT protobuf_msgsTargets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include/protobuf_msgs
)

install(FILES
    ${PROTO_HDRS}
    DESTINATION include/protobuf_msgs
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/protobuf_msgsConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/protobuf_msgsConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/protobuf_msgsConfig.cmake"
    INSTALL_DESTINATION lib/cmake/protobuf_msgs
)

install(EXPORT protobuf_msgsTargets
    FILE protobuf_msgsTargets.cmake
    NAMESPACE protobuf_msgs::
    DESTINATION lib/cmake/protobuf_msgs
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/protobuf_msgsConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/protobuf_msgsConfigVersion.cmake"
    DESTINATION lib/cmake/protobuf_msgs
)