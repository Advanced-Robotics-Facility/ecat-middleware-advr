cmake_minimum_required(VERSION 3.16)

project(advrf_interfaces_protobuf
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

add_library(advrf_interfaces_protobuf STATIC
    ${PROTO_SRCS}
    ${PROTO_HDRS}
)

# add_library(advrf_interfaces_protobuf ALIAS advrf_interfaces_protobuf)

set_target_properties(advrf_interfaces_protobuf
    PROPERTIES
        POSITION_INDEPENDENT_CODE ON
)

target_compile_features(advrf_interfaces_protobuf
    PUBLIC
        cxx_std_17
)

target_include_directories(advrf_interfaces_protobuf
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/advrf_interfaces_protobuf>
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
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/advrf_interfaces_protobuf
)

install(
    FILES ${PROTO_HDRS}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/advrf_interfaces_protobuf
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
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/advrf_interfaces_protobufConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/advrf_interfaces_protobufConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/advrf_interfaces_protobufConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/advrf_interfaces_protobufConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/advrf_interfaces_protobufConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)