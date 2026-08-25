# TODO [Manuel]: 001 and 002 needs to be designed without ecat master dep
#add_subdirectory(001_protobuf)
#if(ZENOH_ROS2_SUPPORT)
#    add_subdirectory(002_cdr)
#endif()
add_subdirectory(003_service)
add_subdirectory(004_subscriber)

# prefix executables with "zenoh_examples_"
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
prefix_executables("${CMAKE_CURRENT_SOURCE_DIR}" "zenoh_examples_")