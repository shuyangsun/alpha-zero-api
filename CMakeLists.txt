cmake_minimum_required(VERSION 3.28.3)

project(AlphaZeroAPI
    VERSION 0.0.1
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -Wall -DNDEBUG")

# =========================== Configuration options ============================
option(STATIC_ONLY "Build static library only." False)
option(SHARED_ONLY "Build shared library only." False)

if(STATIC_ONLY AND SHARED_ONLY)
    message(FATAL_ERROR "STATIC_ONLY and SHARED_ONLY cannot be both True.")
endif()

option(DEFAULT_SERIALIZER "Build with default implementations of ISerializer." True)
option(DEFAULT_DESERIALIZER "Build with default implementation of IDeserializer." True)

configure_file(
    include/alpha-zero-api/configuration.h.in
    include/alpha-zero-api/configuration.h
)

# =============================== Build Targets ================================
add_library(api INTERFACE)
target_include_directories(api INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/include>
)

install(TARGETS api
    EXPORT AlphaZeroAPITargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/include/alpha-zero-api/configuration.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/alpha-zero-api/game.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/alpha-zero-api/policy_output.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/alpha-zero-api/serializer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/alpha-zero-api/deserializer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/alpha-zero-api/augmenter.h
    DESTINATION include/alpha-zero-api
    COMPONENT api
)
export(EXPORT AlphaZeroAPITargets
    FILE "${CMAKE_CURRENT_BINARY_DIR}/AlphaZeroAPITargets.cmake"
    NAMESPACE AlphaZeroAPI::
)
configure_file(cmake/AlphaZeroAPIConfig.cmake
    "${CMAKE_CURRENT_BINARY_DIR}/AlphaZeroAPIConfig.cmake"
    COPYONLY
)

set(ConfigPackageLocation lib/cmake/AlphaZeroAPI)
install(EXPORT AlphaZeroAPITargets
    FILE AlphaZeroAPITargets.cmake
    NAMESPACE AlphaZeroAPI::
    DESTINATION ${ConfigPackageLocation}
)
install(
    FILES
    cmake/AlphaZeroAPIConfig.cmake
    DESTINATION ${ConfigPackageLocation}
    COMPONENT api
)
# export(TARGETS AlphaZeroAPI
#     NAMESPACE AlphaZeroAPI::
#     FILE "${CMAKE_CURRENT_BINARY_DIR}/AlphaZeroAPIConfig.cmake"
# )

# install(EXPORT AlphaZeroAPIConfig
#     DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/lib/cmake/AlphaZeroAPI"
#     NAMESPACE AlphaZeroAPI::
# )

# set_property(TARGET AlphaZeroAPI PROPERTY POSITION_INDEPENDENT_CODE 1)
# if(DEFAULT_SERIALIZER)
#     target_compile_definitions(AlphaZeroAPI PRIVATE "DEFAULT_SERIALIZER")
# endif()

# if(NOT SHARED_ONLY)
#     add_library(AlphaZeroAPI_static STATIC $<TARGET_OBJECTS:AlphaZeroAPI>)
# endif()

# if(NOT STATIC_ONLY)
#     add_library(AlphaZeroAPI_shared SHARED $<TARGET_OBJECTS:AlphaZeroAPI>)
# endif()
