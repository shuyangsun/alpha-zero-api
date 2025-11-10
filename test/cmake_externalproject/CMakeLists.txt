cmake_minimum_required(VERSION 3.28.3)

project(AlphaZeroAPITests-SuperBuild LANGUAGES CXX)

set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -Wall -DNDEBUG")

include(ExternalProject)

ExternalProject_Add(AlphaZeroAPI
    PREFIX AlphaZeroAPI
    SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../.."
    BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/AlphaZeroAPI"
    INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/alpha-zero-api"
    CMAKE_CACHE_ARGS
    -DDEFAULTS:BOOL=ON
        -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_CURRENT_BINARY_DIR}/alpha-zero-api
)

ExternalProject_Add(AlphaZeroAPITests
    PREFIX AlphaZeroAPITests
    SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/.."
    BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/alpha-zero-api-tests"
    BUILD_COMMAND ${CMAKE_COMMAND} --build <BINARY_DIR> --target connect4-main
    INSTALL_COMMAND ""
    CMAKE_CACHE_ARGS
        -DAlphaZeroAPI_DIR:PATH=${CMAKE_CURRENT_BINARY_DIR}/alpha-zero-api/lib/cmake/AlphaZeroAPI
        -DCMAKE_PREFIX_PATH:PATH=${CMAKE_CURRENT_BINARY_DIR}/alpha-zero-api
    DEPENDS AlphaZeroAPI
)
