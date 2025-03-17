cmake_minimum_required(VERSION 3.28.3)

project(LibAlphaZeroAPI LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -Wall -DNDEBUG")

add_library(libalphazero_api OBJECT api/cpp/policy_output.cc)
target_include_directories(libalphazero_api PUBLIC ${PROJECT_SOURCE_DIR})

set_property(TARGET libalphazero_api PROPERTY POSITION_INDEPENDENT_CODE 1)
add_library(libalphazero_api_static STATIC $<TARGET_OBJECTS:libalphazero_api>)
target_include_directories(libalphazero_api PUBLIC ${PROJECT_SOURCE_DIR})
