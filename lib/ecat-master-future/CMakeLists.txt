cmake_minimum_required(VERSION 3.16)

project(ecat-master-future
    VERSION 1.0.0
    LANGUAGES CXX
)

find_package(protobuf_msgs REQUIRED)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

add_library(ecat-master-future SHARED
    src/shm_utils.cpp
    src/shm_shared_types.cpp
)

add_library(ecat-master-future::ecat-master-future ALIAS ecat-master-future)

target_compile_features(ecat-master-future
    PUBLIC
        cxx_std_17
)

target_include_directories(ecat-master-future
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(ecat-master-future
    PUBLIC
        protobuf_msgs::protobuf_msgs
)

set_target_properties(ecat-master-future PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
)

install(
    TARGETS ecat-master-future
    EXPORT ecat-master-futureTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
    DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

option(BUILD_ECAT_MASTER_FUTURE_TOOLS "Build ecat-master-future tools" ON)

if(BUILD_ECAT_MASTER_FUTURE_TOOLS)
    add_executable(ecat_master_future_producer
        src/producer.cpp
    )

    target_compile_features(ecat_master_future_producer
        PRIVATE
            cxx_std_17
    )

    target_link_libraries(ecat_master_future_producer
        PRIVATE
            ecat-master-future::ecat-master-future
            protobuf_msgs::protobuf_msgs
    )

    install(
        TARGETS ecat_master_future_producer
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif()

install(
    EXPORT ecat-master-futureTargets
    FILE ecat-master-futureTargets.cmake
    NAMESPACE ecat-master-future::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/ecat-master-futureConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/ecat-master-futureConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/ecat-master-futureConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/ecat-master-futureConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/ecat-master-futureConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)