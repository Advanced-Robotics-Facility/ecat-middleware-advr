cmake_minimum_required(VERSION 3.16)

project(advrf_cyclonedds_lib
    VERSION 0.1.0
    LANGUAGES C CXX
)

include(ExternalProject)
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(ADVRF_VENDOR_PREFIX
    "${CMAKE_BINARY_DIR}/vendor_install"
)

ExternalProject_Add(cyclonedds_vendor
    GIT_REPOSITORY https://github.com/eclipse-cyclonedds/cyclonedds.git
    GIT_TAG releases/11.0.x

    CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX=${ADVRF_VENDOR_PREFIX}
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DBUILD_EXAMPLES=OFF
        -DBUILD_TESTING=OFF
        -DENABLE_SECURITY=OFF
        -DENABLE_SSL=OFF
        -DENABLE_TYPELIB=ON
)

ExternalProject_Add(cyclonedds_cxx_vendor
    GIT_REPOSITORY https://github.com/eclipse-cyclonedds/cyclonedds-cxx.git
    GIT_TAG releases/11.0.x

    DEPENDS cyclonedds_vendor

    CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX=${ADVRF_VENDOR_PREFIX}
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DCycloneDDS_DIR=${ADVRF_VENDOR_PREFIX}/${CMAKE_INSTALL_LIBDIR}/cmake/CycloneDDS
        -DCMAKE_PREFIX_PATH=${ADVRF_VENDOR_PREFIX}
        -DBUILD_EXAMPLES=OFF
        -DBUILD_TESTING=OFF
)

add_custom_target(advrf_cyclonedds_vendor
    ALL
    DEPENDS cyclonedds_vendor cyclonedds_cxx_vendor
)

install(
    DIRECTORY "${ADVRF_VENDOR_PREFIX}/"
    DESTINATION "."
)

install(CODE [[
    if(EXISTS "${CMAKE_INSTALL_PREFIX}/bin/idlc")
        execute_process(
            COMMAND chmod +x "${CMAKE_INSTALL_PREFIX}/bin/idlc"
        )
    endif()
]])


include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

configure_package_config_file(
    cmake/advrf_cyclonedds_libConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/advrf_cyclonedds_libConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/advrf_cyclonedds_libConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/advrf_cyclonedds_libConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/advrf_cyclonedds_libConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)