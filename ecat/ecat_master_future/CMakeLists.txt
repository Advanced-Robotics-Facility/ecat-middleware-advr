cmake_minimum_required(VERSION 3.16)

project(ecat_master_future
    VERSION 1.0.0
    LANGUAGES CXX
)

find_package(advrf_interfaces_protobuf REQUIRED)
find_package(advrf_middleware_core REQUIRED)
find_package(yaml-cpp REQUIRED)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

add_library(ecat_master_future SHARED
    src/shm_utils.cpp
    src/shm_shared_types.cpp
)

# add_library(advrf::ecat_master_future ALIAS ecat_master_future)

target_compile_features(ecat_master_future
    PUBLIC
        cxx_std_17
)

target_include_directories(ecat_master_future
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)


target_link_libraries(ecat_master_future
    PUBLIC
        advrf_interfaces_protobuf
        advrf_middleware_core
)

set_target_properties(ecat_master_future PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
)

install(
    TARGETS ecat_master_future
    EXPORT ecat_master_futureTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
    DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

option(BUILD_ECAT_MASTER_FUTURE_TOOLS "Build ecat_master_future tools" ON)

if(BUILD_ECAT_MASTER_FUTURE_TOOLS)
    add_executable(ecat_master_future_producer
        src/producer.cpp
    )

    target_compile_definitions(ecat_master_future_producer PRIVATE
        ROBOT_CONFIG_DIR="${CMAKE_CURRENT_SOURCE_DIR}/config/robot.yaml"
    )

    target_compile_features(ecat_master_future_producer
        PRIVATE
            cxx_std_17
    )

    target_link_libraries(ecat_master_future_producer
        PRIVATE
            ecat_master_future
            advrf_interfaces_protobuf
            yaml-cpp::yaml-cpp
    )

    

    install(
        TARGETS ecat_master_future_producer
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif()

install(
    EXPORT ecat_master_futureTargets
    FILE ecat_master_futureTargets.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/ecat_master_futureConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/ecat_master_futureConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/ecat_master_futureConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/ecat_master_futureConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/ecat_master_futureConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

install(
    DIRECTORY config/
    DESTINATION share/${PROJECT_NAME}/config
)

configure_package_config_file(
    cmake/ecat_master_futureConfig.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/ecat_master_futureConfig.cmake"
    INSTALL_DESTINATION lib/cmake/ecat_master_future
)

install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/ecat_master_futureConfig.cmake"
    DESTINATION
        lib/cmake/ecat_master_future
)