cmake_minimum_required(VERSION 3.16)

project(advrf_middleware_core
    VERSION 0.1.0
    LANGUAGES CXX
)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

find_package(advrf_interfaces_protobuf REQUIRED)
find_package(ecat_master_future REQUIRED)
find_package(spdlog REQUIRED)
find_package(yaml-cpp REQUIRED)

add_library(advrf_middleware_core SHARED
    src/middleware.cpp
)

target_compile_features(advrf_middleware_core
    PUBLIC
        cxx_std_17
)

target_include_directories(advrf_middleware_core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(advrf_middleware_core
    PUBLIC
        advrf_interfaces_protobuf
        ecat_master_future
        spdlog::spdlog
        yaml-cpp::yaml-cpp
)

install(
    TARGETS advrf_middleware_core
    EXPORT advrf_middleware_coreTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
    DIRECTORY
        ${CMAKE_CURRENT_SOURCE_DIR}/include/
    DESTINATION
        ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
    DIRECTORY
        config/
    DESTINATION
        share/${PROJECT_NAME}/config
)

install(
    EXPORT advrf_middleware_coreTargets
    FILE advrf_middleware_coreTargets.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/advrf_middleware_coreConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/advrf_middleware_coreConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/advrf_middleware_coreConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/advrf_middleware_coreConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/advrf_middleware_coreConfigVersion.cmake
    DESTINATION
        ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

 add_executable(ecat_master_future_producer
        src/producer.cpp
)

target_compile_definitions(ecat_master_future_producer PRIVATE
    ROBOT_CONFIG_DIR="${CMAKE_INSTALL_FULL_DATADIR}/${PROJECT_NAME}/config/robot.yaml"
)

target_compile_features(ecat_master_future_producer
    PRIVATE
        cxx_std_17
)

target_link_libraries(ecat_master_future_producer
    PRIVATE
        advrf_middleware_core
        ecat_master_future
        advrf_interfaces_protobuf
        yaml-cpp::yaml-cpp
)

install(
    TARGETS ecat_master_future_producer
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)