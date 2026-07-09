cmake_minimum_required(VERSION 3.16)

project(protobuf_msgs
    VERSION 1.0.0
    LANGUAGES CXX
)

find_package(Protobuf REQUIRED)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(PROTO_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/msg/time.proto
    ${CMAKE_CURRENT_SOURCE_DIR}/msg/msg_header.proto
    ${CMAKE_CURRENT_SOURCE_DIR}/msg/motor_pdo.proto
    ${CMAKE_CURRENT_SOURCE_DIR}/msg/imu_pdo.proto
    ${CMAKE_CURRENT_SOURCE_DIR}/msg/ecat_pdo.proto
    ${CMAKE_CURRENT_SOURCE_DIR}/msg/ft6_pdo.proto
    ${CMAKE_CURRENT_SOURCE_DIR}/msg/power_pack_pdo.proto
    ${CMAKE_CURRENT_SOURCE_DIR}/msg/cia402.proto
    ${CMAKE_CURRENT_SOURCE_DIR}/msg/hyq.proto
    ${CMAKE_CURRENT_SOURCE_DIR}/msg/generic.proto
)

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILES})

add_library(protobuf_msgs STATIC
    ${PROTO_SRCS}
    ${PROTO_HDRS}
)

add_library(protobuf_msgs::protobuf_msgs ALIAS protobuf_msgs)

set_target_properties(protobuf_msgs
    PROPERTIES
        POSITION_INDEPENDENT_CODE ON
)

target_compile_features(protobuf_msgs
    PUBLIC
        cxx_std_17
)

target_include_directories(protobuf_msgs
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/protobuf_msgs>
)

target_link_libraries(protobuf_msgs
    PUBLIC
        protobuf::libprotobuf
)

install(
    TARGETS protobuf_msgs
    EXPORT protobuf_msgsTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/protobuf_msgs
)

install(
    FILES ${PROTO_HDRS}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/protobuf_msgs
)

install(
    FILES ${PROTO_FILES}
    DESTINATION share/${PROJECT_NAME}/proto
)

install(
    EXPORT protobuf_msgsTargets
    FILE protobuf_msgsTargets.cmake
    NAMESPACE protobuf_msgs::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/protobuf_msgsConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/protobuf_msgsConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/protobuf_msgsConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/protobuf_msgsConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/protobuf_msgsConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)