cmake_minimum_required(VERSION 3.16)

project(advrf_middleware_core
    VERSION 0.1.0
    LANGUAGES CXX
)

find_package(advrf_interfaces_protobuf REQUIRED)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

add_library(advrf_middleware_core INTERFACE)
# add_library(advrf_middleware_core ALIAS mw_core)

target_compile_features(advrf_middleware_core
    INTERFACE
        cxx_std_17
)

target_include_directories(advrf_middleware_core
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(advrf_middleware_core
    INTERFACE
        advrf_interfaces_protobuf
)
install(
    TARGETS advrf_middleware_core
    EXPORT advrf_middleware_coreTargets
)

install(
    DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
    EXPORT advrf_middleware_coreTargets
    FILE advrf_middleware_coreTargets.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

configure_package_config_file(
    cmake/advrf_middleware_coreConfig.cmake.in
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
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)