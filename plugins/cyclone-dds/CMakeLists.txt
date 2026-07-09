cmake_minimum_required(VERSION 3.16)

project(ecat_plugin_cyclonedds
    VERSION 0.1.0
    LANGUAGES CXX
)

find_package(advrf_cyclonedds_lib REQUIRED)
find_package(advrf_interfaces_dds REQUIRED)
find_package(advrf_middleware_core REQUIRED)
find_package(protobuf_msgs REQUIRED)
find_package(ecat-master-future REQUIRED)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

add_library(ecat_plugin_cyclonedds
    src/main.cpp
    src/parameter_make_get.cpp
    src/service.cpp
    src/parameter_client.cpp
    src/parameter_server.cpp
)

add_library(ecat::plugin_cyclonedds ALIAS ecat_plugin_cyclonedds)

target_compile_features(ecat_plugin_cyclonedds
    PUBLIC
        cxx_std_17
)

target_include_directories(ecat_plugin_cyclonedds
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(ecat_plugin_cyclonedds
    PUBLIC
        advrf::mw_core
        advrf::advrf_dds_interfaces
        protobuf_msgs::protobuf_msgs
        ecat-master-future::ecat-master-future
        CycloneDDS-CXX::ddscxx
)
install(
    TARGETS ecat_plugin_cyclonedds
    EXPORT ecat_plugin_cycloneddsTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
    DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
    EXPORT ecat_plugin_cycloneddsTargets
    FILE ecat_plugin_cycloneddsTargets.cmake
    NAMESPACE ecat::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/ecat_plugin_cycloneddsConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/ecat_plugin_cycloneddsConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/ecat_plugin_cycloneddsConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/ecat_plugin_cycloneddsConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/ecat_plugin_cycloneddsConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

option(BUILD_ECAT_PLUGIN_CYCLONEDDS_TESTS "Build ecat_plugin_cyclonedds tests" ON)

if(BUILD_ECAT_PLUGIN_CYCLONEDDS_TESTS)
    enable_testing()

    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/test/CMakeLists.txt")
        add_subdirectory(test)
    endif()

    # add_executable(publisher
    #     src/publisher.cpp
    # )

    # target_compile_features(publisher
    #     PRIVATE
    #         cxx_std_17
    # )

    # target_include_directories(publisher
    #     PRIVATE
    #         ${CMAKE_CURRENT_SOURCE_DIR}/include
    # )

    # target_link_libraries(publisher
    #     PRIVATE
    #         ecat::plugin_cyclonedds
    # )
endif()