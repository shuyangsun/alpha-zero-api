cmake_minimum_required(VERSION 3.28.3)

project(AlphaZeroAPI
    VERSION 0.0.1
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -Wall -DNDEBUG")

# ================================== Options ===================================

option(STATIC_ONLY "Build static library only." False)
option(SHARED_ONLY "Build shared library only." False)

if(STATIC_ONLY AND SHARED_ONLY)
    message(FATAL_ERROR "STATIC_ONLY and SHARED_ONLY cannot be both True.")
endif()

option(DEFAULTS "Build with default implementations for serializer and deserializer." False)

# ============================== Component: API ================================

configure_file(
    include/alpha-zero-api/configuration.h.in
    include/alpha-zero-api/configuration.h
)
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

# ============================ Component: Defaults =============================

if(DEFAULTS)
    add_library(defaults INTERFACE)
    target_include_directories(defaults INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
        $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/include>
    )
    install(TARGETS defaults
        EXPORT AlphaZeroAPITargets
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
        INCLUDES DESTINATION include
    )
    install(FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/include/alpha-zero-api/defaults/serializer.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/alpha-zero-api/defaults/deserializer.h
        DESTINATION include/alpha-zero-api/defaults
        COMPONENT defaults
    )
endif() # DEFAULTS

# ============================ CMake Config Files ==============================

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

if(DEFAULTS)
    install(
        FILES
        cmake/AlphaZeroAPIConfig.cmake
        DESTINATION ${ConfigPackageLocation}
        COMPONENT defaults
    )
endif() # DEFAULTS

# if(NOT SHARED_ONLY)
#     add_library(AlphaZeroAPI_static STATIC $<TARGET_OBJECTS:AlphaZeroAPI>)
# endif()

# if(NOT STATIC_ONLY)
#     add_library(AlphaZeroAPI_shared SHARED $<TARGET_OBJECTS:AlphaZeroAPI>)
# endif()
