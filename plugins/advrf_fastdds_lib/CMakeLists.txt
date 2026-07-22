cmake_minimum_required(VERSION 3.16)

project(advrf_fastdds_lib
    VERSION 0.1.0
    LANGUAGES C CXX
)

include(ExternalProject)
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(ADVRF_VENDOR_PREFIX
    "${CMAKE_BINARY_DIR}/vendor_install"
)

#
# foonathan_memory_vendor
#
ExternalProject_Add(foonathan_memory_vendor
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/foonathan_memory_vendor

    DOWNLOAD_COMMAND ""
    UPDATE_COMMAND ""
    PATCH_COMMAND ""

    CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX=${ADVRF_VENDOR_PREFIX}
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
)

#
# Fast-CDR
#
ExternalProject_Add(fastcdr_vendor
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/Fast-CDR

    DOWNLOAD_COMMAND ""
    UPDATE_COMMAND ""
    PATCH_COMMAND ""

    CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX=${ADVRF_VENDOR_PREFIX}
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
)

#
# Fast-DDS
#
ExternalProject_Add(fastdds_vendor
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/Fast-DDS

    DEPENDS
        foonathan_memory_vendor
        fastcdr_vendor

    DOWNLOAD_COMMAND ""
    UPDATE_COMMAND ""
    PATCH_COMMAND ""

    CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX=${ADVRF_VENDOR_PREFIX}
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DCMAKE_PREFIX_PATH=${ADVRF_VENDOR_PREFIX}
        -DBUILD_TESTING=OFF
)

#
# Fast-DDS-Gen
ExternalProject_Add(fastddsgen_vendor
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/Fast-DDS-Gen

    DOWNLOAD_COMMAND ""
    UPDATE_COMMAND ""
    PATCH_COMMAND ""

    CONFIGURE_COMMAND ""

    BUILD_IN_SOURCE TRUE

    BUILD_COMMAND
        ./gradlew build -x test

    INSTALL_COMMAND
        ""
)

add_custom_target(advrf_fastdds_lib
    ALL
    DEPENDS
        foonathan_memory_vendor
        fastcdr_vendor
        fastdds_vendor
        fastddsgen_vendor
)

#
# Install Fast-DDS libraries
#
install(
    DIRECTORY "${ADVRF_VENDOR_PREFIX}/"
    DESTINATION "."
)

#
# Install Fast-DDS-Gen
#
install(
    DIRECTORY
        "${CMAKE_CURRENT_SOURCE_DIR}/lib/Fast-DDS-Gen/build/libs/"
    DESTINATION
        "share/${PROJECT_NAME}/fastddsgen"
    OPTIONAL
)

#
# Install Gradle wrapper and runtime files
#
install(
    PROGRAMS
        "${CMAKE_CURRENT_SOURCE_DIR}/lib/Fast-DDS-Gen/gradlew"
    DESTINATION
        "share/${PROJECT_NAME}/fastddsgen"
)

install(
    DIRECTORY
        "${CMAKE_CURRENT_SOURCE_DIR}/lib/Fast-DDS-Gen/gradle"
    DESTINATION
        "share/${PROJECT_NAME}/fastddsgen"
    OPTIONAL
)

configure_package_config_file(
  cmake/advrf_fastdds_libConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/advrf_fastdds_libConfig.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/advrf_fastdds_libConfigVersion.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
)

install(
  FILES
    ${CMAKE_CURRENT_BINARY_DIR}/advrf_fastdds_libConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/advrf_fastdds_libConfigVersion.cmake
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)