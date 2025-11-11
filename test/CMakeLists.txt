cmake_minimum_required(VERSION 3.28.3)

project(AlphaZeroAPITests LANGUAGES CXX)

set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -Wall -DNDEBUG")

find_package(AlphaZeroAPI CONFIG REQUIRED COMPONENTS api defaults)

add_library(alpha-zero-api-test-lib STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/tic_tac_toe/serializer.cc
)
target_link_libraries(alpha-zero-api-test-lib PUBLIC AlphaZeroAPI::api AlphaZeroAPI::defaults)
target_include_directories(alpha-zero-api-test-lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PROJECT_BINARY_DIR}
)

add_executable(tic-tac-toe-main
    ${CMAKE_CURRENT_SOURCE_DIR}/tic_tac_toe/main.cc
)
target_link_libraries(tic-tac-toe-main PRIVATE alpha-zero-api-test-lib)
target_include_directories(tic-tac-toe-main PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PROJECT_BINARY_DIR}
)
