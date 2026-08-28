# add subdirectories for each example
add_subdirectory(001_publisher)
add_subdirectory(002_service)
add_subdirectory(003_subscriber)


# prefix executables with "cyclonedds_examples_"
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
prefix_executables("${CMAKE_CURRENT_SOURCE_DIR}" "cyclonedds_examples_")