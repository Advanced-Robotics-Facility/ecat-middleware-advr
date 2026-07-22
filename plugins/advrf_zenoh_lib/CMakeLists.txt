cmake_minimum_required(VERSION 3.16)

project(advrf_zenoh_lib
    VERSION 0.1.0
    LANGUAGES C CXX
)

include(ExternalProject)
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

find_program(CARGO_EXECUTABLE cargo)
find_program(RUSTC_EXECUTABLE rustc)

if(NOT CARGO_EXECUTABLE OR NOT RUSTC_EXECUTABLE)
  message(FATAL_ERROR "advrf_zenoh_lib requires a Rust toolchain (cargo/rustc) to build zenoh-c.\n")
endif()

set(ADVRF_VENDOR_PREFIX
    "${CMAKE_BINARY_DIR}/vendor_install"
)

option(ADVRF_ZENOHC_UNSTABLE_API "Enable zenoh-c unstable API" OFF)
option(ADVRF_ZENOHC_SHARED_MEMORY "Enable zenoh-c shared memory support" OFF)

ExternalProject_Add(zenoh_c_vendor
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/zenoh-c

    DOWNLOAD_COMMAND ""
    UPDATE_COMMAND ""
    PATCH_COMMAND ""

    CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX=${ADVRF_VENDOR_PREFIX}
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DZENOHC_BUILD_WITH_UNSTABLE_API=${ADVRF_ZENOHC_UNSTABLE_API}
        -DZENOHC_BUILD_WITH_SHARED_MEMORY=${ADVRF_ZENOHC_SHARED_MEMORY}
)

ExternalProject_Add(zenoh_cpp_vendor
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/zenoh-cpp

    DEPENDS zenoh_c_vendor

    DOWNLOAD_COMMAND ""
    UPDATE_COMMAND ""
    PATCH_COMMAND ""

    CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX=${ADVRF_VENDOR_PREFIX}
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -Dzenohc_DIR=${ADVRF_VENDOR_PREFIX}/${CMAKE_INSTALL_LIBDIR}/cmake/zenohc
        -DCMAKE_PREFIX_PATH=${ADVRF_VENDOR_PREFIX}
        -DZENOHCXX_ZENOHC=ON
        -DZENOHCXX_ZENOHPICO=OFF
)

add_custom_target(advrf_zenoh_vendor
    ALL
    DEPENDS
        zenoh_c_vendor
        zenoh_cpp_vendor
)

install(
    DIRECTORY "${ADVRF_VENDOR_PREFIX}/"
    DESTINATION "."
)

configure_package_config_file(
    cmake/advrf_zenoh_libConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/advrf_zenoh_libConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/advrf_zenoh_libConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/advrf_zenoh_libConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/advrf_zenoh_libConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)