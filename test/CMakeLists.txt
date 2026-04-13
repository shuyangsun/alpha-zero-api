cmake_minimum_required(VERSION 3.31.6)

project(AlphaZeroAPITests LANGUAGES CXX)

set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -Wall -DNDEBUG")

include(FetchContent)

set(DEFAULTS
    ON
    CACHE BOOL "" FORCE)

FetchContent_Declare(AlphaZeroAPI SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/..")

FetchContent_MakeAvailable(AlphaZeroAPI)

add_library(game STATIC ${CMAKE_CURRENT_SOURCE_DIR}/tic_tac_toe/game.cc)
target_link_libraries(game PUBLIC AlphaZeroAPI::api AlphaZeroAPI::defaults)
target_include_directories(game PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${PROJECT_BINARY_DIR})

add_library(serializer STATIC ${CMAKE_CURRENT_SOURCE_DIR}/tic_tac_toe/serializer.cc)
target_link_libraries(serializer PUBLIC AlphaZeroAPI::api AlphaZeroAPI::defaults)
target_include_directories(serializer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${PROJECT_BINARY_DIR})

add_library(deserializer STATIC ${CMAKE_CURRENT_SOURCE_DIR}/tic_tac_toe/deserializer.cc)
target_link_libraries(deserializer PUBLIC AlphaZeroAPI::api AlphaZeroAPI::defaults)
target_include_directories(deserializer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${PROJECT_BINARY_DIR})

add_library(augmentation STATIC ${CMAKE_CURRENT_SOURCE_DIR}/tic_tac_toe/augmentation.cc)
target_link_libraries(augmentation PUBLIC AlphaZeroAPI::api AlphaZeroAPI::defaults game)
target_include_directories(augmentation PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${PROJECT_BINARY_DIR})

add_library(inference STATIC ${CMAKE_CURRENT_SOURCE_DIR}/tic_tac_toe/inference.cc)
target_link_libraries(inference PUBLIC AlphaZeroAPI::api AlphaZeroAPI::defaults augmentation)
target_include_directories(inference PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${PROJECT_BINARY_DIR})

add_library(train STATIC ${CMAKE_CURRENT_SOURCE_DIR}/tic_tac_toe/train.cc)
target_link_libraries(train PUBLIC AlphaZeroAPI::api AlphaZeroAPI::defaults augmentation)
target_include_directories(train PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${PROJECT_BINARY_DIR})

add_executable(tic-tac-toe-main ${CMAKE_CURRENT_SOURCE_DIR}/tic_tac_toe/main.cc)
target_link_libraries(tic-tac-toe-main PRIVATE serializer deserializer inference train game)
target_include_directories(tic-tac-toe-main PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
                                                    ${PROJECT_BINARY_DIR})
