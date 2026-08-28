add_subdirectory(001_publisher)
add_subdirectory(002_subscriber)
add_subdirectory(003_service)


MESSAGE(STATUS "FASTDDSEXAMPLES \n\n\n Build type: ${CMAKE_BUILD_TYPE}")

# prefix executables with "fastdds_examples_"
function(prefix_executables dir prefix)
    get_property(targets DIRECTORY "${dir}" PROPERTY BUILDSYSTEM_TARGETS)
    foreach(target IN LISTS targets)
        get_target_property(type "${target}" TYPE)
        if(type STREQUAL "EXECUTABLE")
            set_target_properties("${target}" PROPERTIES
                OUTPUT_NAME "${prefix}${target}"
            )
        endif()
    endforeach()
    get_property(subdirs DIRECTORY "${dir}" PROPERTY SUBDIRECTORIES)

    foreach(subdir IN LISTS subdirs)
        prefix_executables("${subdir}" "${prefix}")
    endforeach()
endfunction()
prefix_executables("${CMAKE_CURRENT_SOURCE_DIR}" "fastdds_examples_")