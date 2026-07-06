unset(DDS_MSG_IDL_FILES CACHE)
unset(DDS_SERVICE_IDL_FILES CACHE)
unset(DDS_IDL_DIR CACHE)

set(DDS_IDL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/idl")

file(GLOB DDS_MSG_IDL_FILES
    "${DDS_IDL_DIR}/messages/*.idl"
)

file(GLOB DDS_SERVICE_IDL_FILES
    "${DDS_IDL_DIR}/services/*.idl"
)

if(NOT DDS_MSG_IDL_FILES)
    message(FATAL_ERROR "No message IDL files found")
endif()

if(NOT DDS_SERVICE_IDL_FILES)
    message(STATUS "No service IDL files found")
endif()

set(DDS_MSG_IDL_FILES
    "${DDS_MSG_IDL_FILES}"
    CACHE INTERNAL ""
    FORCE
)

set(DDS_SERVICE_IDL_FILES
    "${DDS_SERVICE_IDL_FILES}"
    CACHE INTERNAL ""
    FORCE
)

set(DDS_IDL_DIR
    "${DDS_IDL_DIR}"
    CACHE INTERNAL ""
    FORCE
)

message(STATUS "DDS Message IDLs:")
foreach(idl ${DDS_MSG_IDL_FILES})
    message(STATUS "  - ${idl}")
endforeach()

message(STATUS "DDS Service IDLs:")
foreach(idl ${DDS_SERVICE_IDL_FILES})
    message(STATUS "  - ${idl}")
endforeach()