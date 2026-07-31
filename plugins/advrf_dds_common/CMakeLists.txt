cmake_minimum_required(VERSION 3.16)

project(advrf_dds_common
    VERSION 0.1.0
    LANGUAGES CXX
)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

find_package(advrf_interfaces_dds REQUIRED)
find_package(advrf_interfaces_protobuf REQUIRED)

set(ADVRF_DDS_COMMON_TARGETS "")

function(advrf_dds_common_add_backend backend)
    set(interface_target "advrf_interfaces_dds::${backend}")

    if(NOT TARGET "${interface_target}")
        return()
    endif()

    set(target "advrf_dds_common_${backend}")
    add_library("${target}" STATIC src/converter.cpp)
    set_target_properties("${target}" PROPERTIES
        EXPORT_NAME "${backend}"
        POSITION_INDEPENDENT_CODE ON
    )
    target_compile_features("${target}" PUBLIC cxx_std_17)
    target_include_directories("${target}"
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )
    target_link_libraries("${target}"
        PUBLIC
            "${interface_target}"
            advrf_interfaces_protobuf
    )

    install(
        TARGETS "${target}"
        EXPORT advrf_dds_commonTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )

    set(ADVRF_DDS_COMMON_TARGETS
        "${ADVRF_DDS_COMMON_TARGETS};${target}"
        PARENT_SCOPE
    )
endfunction()

advrf_dds_common_add_backend(fastdds)
advrf_dds_common_add_backend(cyclonedds)

if(NOT ADVRF_DDS_COMMON_TARGETS)
    message(FATAL_ERROR
        "advrf_interfaces_dds does not provide an enabled DDS backend target."
    )
endif()

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(
    EXPORT advrf_dds_commonTargets
    FILE advrf_dds_commonTargets.cmake
    NAMESPACE advrf_dds_common::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

configure_package_config_file(
    cmake/advrf_dds_commonConfig.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/advrf_dds_commonConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/advrf_dds_commonConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/advrf_dds_commonConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/advrf_dds_commonConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)
