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
    include/alpha-zero-api/game.h
    include/alpha-zero-api/policy_output.h
    include/alpha-zero-api/serializer.h
    include/alpha-zero-api/deserializer.h
    include/alpha-zero-api/augmenter.h
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
        include/alpha-zero-api/defaults/serializer.h
        include/alpha-zero-api/defaults/deserializer.h
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
