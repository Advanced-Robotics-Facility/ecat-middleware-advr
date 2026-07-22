cmake_minimum_required(VERSION 3.16)

project(advrf_zmq_lib
    VERSION 0.1.0
    LANGUAGES C CXX
)

include(ExternalProject)
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBZMQ REQUIRED libzmq>=4)

set(ADVRF_VENDOR_PREFIX
    "${CMAKE_BINARY_DIR}/vendor_install"
)

ExternalProject_Add(cppzmq_vendor
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/cppzmq

    DOWNLOAD_COMMAND ""
    UPDATE_COMMAND ""
    PATCH_COMMAND ""

    CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX=${ADVRF_VENDOR_PREFIX}
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DPKG_CONFIG_EXECUTABLE=${PKG_CONFIG_EXECUTABLE}
        -DCPPZMQ_BUILD_TESTS=OFF
)

add_custom_target(advrf_zmq_vendor
    ALL
    DEPENDS
        cppzmq_vendor
)

install(
    DIRECTORY "${ADVRF_VENDOR_PREFIX}/"
    DESTINATION "."
)

configure_package_config_file(
    cmake/advrf_zmq_libConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/advrf_zmq_libConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/advrf_zmq_libConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/advrf_zmq_libConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/advrf_zmq_libConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)
