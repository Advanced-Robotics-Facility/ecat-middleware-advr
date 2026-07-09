add_library(mw_core INTERFACE)

target_include_directories(mw_core
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(mw_core
    INTERFACE
        protobuf_msgs
)
