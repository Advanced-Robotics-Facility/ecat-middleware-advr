cmake_minimum_required(VERSION 3.16)

project(advrf_interfaces_protobuf
    VERSION 1.0.0
    LANGUAGES CXX
)

find_package(Protobuf REQUIRED)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(PROTO_FILES
    msg/time.proto
    msg/msg_header.proto
    msg/motor_pdo.proto
    msg/imu_pdo.proto
    msg/ecat_pdo.proto
    msg/ft6_pdo.proto
    msg/power_pack_pdo.proto
    msg/cia402.proto
    msg/hyq.proto
    msg/generic.proto
    msg/gripper_pdo.proto
    msg/repl_cmd.proto
)

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILES})

add_library(advrf_interfaces_protobuf STATIC
    ${PROTO_SRCS}
    ${PROTO_HDRS}
)

set_target_properties(advrf_interfaces_protobuf PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)

target_compile_features(advrf_interfaces_protobuf
    PUBLIC
        cxx_std_17
)

target_include_directories(advrf_interfaces_protobuf
    PUBLIC
        # build/<pkg>/ecat_pdo.pb.h
        # => include <advrf_interfaces_protobuf/ecat_pdo.pb.h>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/..>

        # install/include/advrf_interfaces_protobuf/ecat_pdo.pb.h
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(advrf_interfaces_protobuf
    PUBLIC
        protobuf::libprotobuf
)

install(
    TARGETS advrf_interfaces_protobuf
    EXPORT advrf_interfaces_protobufTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
    DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/advrf_interfaces_protobuf
    FILES_MATCHING
        PATTERN "*.pb.h"
)

install(
    FILES ${PROTO_FILES}
    DESTINATION share/${PROJECT_NAME}/proto
)

install(
    EXPORT advrf_interfaces_protobufTargets
    FILE advrf_interfaces_protobufTargets.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

configure_package_config_file(
    cmake/advrf_interfaces_protobufConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/advrf_interfaces_protobufConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/advrf_interfaces_protobufConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/advrf_interfaces_protobufConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/advrf_interfaces_protobufConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)