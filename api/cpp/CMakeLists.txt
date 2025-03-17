add_library(libalphazero_api OBJECT policy_output.cc)
target_include_directories(libalphazero_api PUBLIC ${PROJECT_SOURCE_DIR})

set_property(TARGET libalphazero_api PROPERTY POSITION_INDEPENDENT_CODE 1)
add_library(libalphazero_api_static STATIC $<TARGET_OBJECTS:libalphazero_api>)
target_include_directories(libalphazero_api PUBLIC ${PROJECT_SOURCE_DIR})
