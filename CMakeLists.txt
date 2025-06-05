cmake_minimum_required(VERSION 3.16)

# Project definition
project(NecrosisEngine 
    VERSION 1.0.0
    DESCRIPTION "High-performance 2D sand simulation engine"
    LANGUAGES CXX
)

# Set C++ standard (as specified in PRD: C++17 or newer)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build configuration options
option(NECROSIS_BUILD_TESTS "Build test executables" ON)
option(NECROSIS_BUILD_EXAMPLES "Build example applications" ON)
option(NECROSIS_BUILD_SHARED "Build shared library instead of static" OFF)
option(NECROSIS_ENABLE_PROFILING "Enable profiling support" OFF)

# Set default build type to Release if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# Platform-specific configurations
if(WIN32)
    add_definitions(-DNECROSIS_PLATFORM_WINDOWS)
    # Enable multi-processor compilation on MSVC
    if(MSVC)
        add_compile_options(/MP)
    endif()
elseif(APPLE)
    add_definitions(-DNECROSIS_PLATFORM_MACOS)
elseif(UNIX)
    add_definitions(-DNECROSIS_PLATFORM_LINUX)
endif()

# Compiler-specific flags
if(MSVC)
    # MSVC specific flags (applied to all configurations)
    add_compile_options(
        /W4                     # High warning level
        /permissive-           # Disable non-conforming code
        /Zc:__cplusplus        # Enable correct __cplusplus macro
    )

    # Release-specific flags
    add_compile_options($<$<CONFIG:Release>:/O2>)
    add_compile_options($<$<CONFIG:Release>:/Ob2>)
    add_compile_options($<$<CONFIG:Release>:/DNDEBUG>)

    # Debug-specific flags
    add_compile_options($<$<CONFIG:Debug>:/Od>)
    add_compile_options($<$<CONFIG:Debug>:/Zi>)
    add_compile_options($<$<CONFIG:Debug>:/RTC1>)

else()
    # GCC/Clang flags
    add_compile_options(
        -Wall -Wextra -Wpedantic
        -Wno-unused-parameter
    )

    # Release optimizations
    add_compile_options($<$<CONFIG:Release>:-O3>)
    add_compile_options($<$<CONFIG:Release>:-DNDEBUG>)
    add_compile_options($<$<CONFIG:Release>:-march=native>)

    # Debug configurations
    add_compile_options($<$<CONFIG:Debug>:-g>)
    add_compile_options($<$<CONFIG:Debug>:-O0>)

    # Enable threading support
    find_package(Threads REQUIRED)
endif()

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# For now, build a simple executable with main.cpp
# As the project grows, this will be refactored into separate engine library + executable

# Current source files
set(NECROSIS_SOURCES
    src/main.cpp
)

# Create the main executable
add_executable(necrosis_engine ${NECROSIS_SOURCES})

# Set target properties
set_target_properties(necrosis_engine PROPERTIES
    OUTPUT_NAME "necrosis"
)

# Link threading library on Unix systems
if(UNIX AND NOT APPLE)
    target_link_libraries(necrosis_engine PRIVATE Threads::Threads)
endif()

# Future engine library structure (commented out for now):
# When you have more source files, uncomment and modify this section:
#
# set(NECROSIS_ENGINE_SOURCES
#     src/core/Engine.cpp
#     src/simulation/PixelGrid.cpp
#     src/simulation/MaterialSystem.cpp
#     src/physics/RigidBodySystem.cpp
#     src/physics/PhysicsWorld.cpp
#     src/rendering/RenderSystem.cpp
#     src/input/InputManager.cpp
#     src/utils/ThreadPool.cpp
#     src/utils/Memory.cpp
# )
#
# set(NECROSIS_ENGINE_HEADERS
#     include/necrosis/Engine.h
#     include/necrosis/PixelGrid.h
#     include/necrosis/MaterialSystem.h
#     include/necrosis/RigidBodySystem.h
#     include/necrosis/PhysicsWorld.h
#     include/necrosis/RenderSystem.h
#     include/necrosis/InputManager.h
#     include/necrosis/ThreadPool.h
#     include/necrosis/Memory.h
# )
#
# if(NECROSIS_BUILD_SHARED)
#     add_library(necrosis_engine_lib SHARED ${NECROSIS_ENGINE_SOURCES} ${NECROSIS_ENGINE_HEADERS})
#     target_compile_definitions(necrosis_engine_lib PRIVATE NECROSIS_EXPORTS)
#     target_compile_definitions(necrosis_engine_lib PUBLIC NECROSIS_SHARED)
# else()
#     add_library(necrosis_engine_lib STATIC ${NECROSIS_ENGINE_SOURCES} ${NECROSIS_ENGINE_HEADERS})
# endif()
#
# add_executable(necrosis_test src/main.cpp)
# target_link_libraries(necrosis_test PRIVATE necrosis_engine_lib)

# Future dependency management (commented out until needed)
# These will be uncommented and configured as development progresses

# SDL2 for input and audio (as specified in PRD)
# find_package(SDL2 REQUIRED)
# target_link_libraries(necrosis_engine PRIVATE SDL2::SDL2 SDL2::SDL2main)

# Graphics framework options (The-Forge, BGFX, etc.)
# These will be added based on the chosen graphics backend

# OpenGL support (initial backend as per PRD)
# find_package(OpenGL REQUIRED)
# target_link_libraries(necrosis_engine PRIVATE OpenGL::GL)

# DirectX support (Windows)
# if(WIN32)
#     target_link_libraries(necrosis_engine PRIVATE d3d11 dxgi)
# endif()

# Set working directory for debugging
set_target_properties(necrosis_engine PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)

# Example applications
if(NECROSIS_BUILD_EXAMPLES)
    # Future example applications will be added here
    # add_subdirectory(examples)
endif()

# Installation configuration
install(TARGETS necrosis_engine
    RUNTIME DESTINATION bin
)

# Future: Install headers when engine becomes a library
# install(DIRECTORY include/
#     DESTINATION include
#     FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp"
# )

# CPack configuration for packaging
set(CPACK_PACKAGE_NAME "NecrosisEngine")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${PROJECT_DESCRIPTION})
set(CPACK_PACKAGE_VENDOR "Necrosis Engine Development Team")

include(CPack)

# Print configuration summary
message(STATUS "=== Necrosis Engine Configuration ===")
message(STATUS "Version: ${PROJECT_VERSION}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Build shared library: ${NECROSIS_BUILD_SHARED}")
message(STATUS "Build tests: ${NECROSIS_BUILD_TESTS}")
message(STATUS "Build examples: ${NECROSIS_BUILD_EXAMPLES}")
message(STATUS "Platform: ${CMAKE_SYSTEM_NAME}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "=====================================")
