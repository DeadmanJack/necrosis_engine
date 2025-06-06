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

# Feature configuration options (for user control)
# These define the cached, user-configurable options.
# Defaults are set for a typical debug-first workflow.
option(NECROSIS_ENABLE_CHEAT_SYSTEM "Enable cheat system" ON)
option(NECROSIS_ENABLE_DEBUG_OVERLAY "Enable debug overlay system" ON)
option(NECROSIS_ENABLE_CONSOLE_COMMANDS "Enable console command system" ON)
option(NECROSIS_ENABLE_PERFORMANCE_METRICS "Enable performance metrics collection" OFF)

# Determine the EFFECTIVE settings for this build based on build type.
# This allows us to force-disable debug features in Release builds,
# regardless of what the cached option() is set to.
set(EFFECTIVE_CHEAT_SYSTEM ${NECROSIS_ENABLE_CHEAT_SYSTEM})
set(EFFECTIVE_DEBUG_OVERLAY ${NECROSIS_ENABLE_DEBUG_OVERLAY})
set(EFFECTIVE_CONSOLE_COMMANDS ${NECROSIS_ENABLE_CONSOLE_COMMANDS})
# Performance metrics can be enabled in any build, so we just use its value directly.
set(EFFECTIVE_PERFORMANCE_METRICS ${NECROSIS_ENABLE_PERFORMANCE_METRICS})

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    # In Release mode, always disable these features.
    set(EFFECTIVE_CHEAT_SYSTEM OFF)
    set(EFFECTIVE_DEBUG_OVERLAY OFF)
    set(EFFECTIVE_CONSOLE_COMMANDS OFF)
endif()

# Pass the effective values to the preprocessor.
# We define the macros only when features are enabled (not when OFF)
if(EFFECTIVE_CHEAT_SYSTEM)
    add_compile_definitions(NECROSIS_CHEAT_SYSTEM_ENABLED=1)
endif()

if(EFFECTIVE_DEBUG_OVERLAY)
    add_compile_definitions(NECROSIS_DEBUG_OVERLAY_ENABLED=1)
endif()

if(EFFECTIVE_CONSOLE_COMMANDS)
    add_compile_definitions(NECROSIS_CONSOLE_COMMANDS_ENABLED=1)
endif()

if(EFFECTIVE_PERFORMANCE_METRICS)
    add_compile_definitions(NECROSIS_PERFORMANCE_METRICS_ENABLED=1)
endif()

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# NOTE: The configure_file() step is no longer needed if you use add_compile_definitions()
# as shown above, which is often simpler. If you prefer the header, you can keep it,
# but make sure it uses the EFFECTIVE_ variables. I will comment it out in favor
# of the more direct add_compile_definitions approach.

# configure_file(
#     "${CMAKE_CURRENT_SOURCE_DIR}/include/necrosis/features.h.in"
#     "${CMAKE_CURRENT_BINARY_DIR}/include/necrosis/features.h"
#     @ONLY
# )
# include_directories(${CMAKE_CURRENT_BINARY_DIR}/include)

# Current source files
set(NECROSIS_SOURCES
    src/main.cpp
    # Note: I removed the duplicate addition of cheat.cpp
)

# Conditionally add feature-specific source files using the effective variable
if(EFFECTIVE_CHEAT_SYSTEM)
    list(APPEND NECROSIS_SOURCES
        src/cheat-system/cheat.cpp
    )
endif()

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

# ... (rest of your file is fine) ...

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
message(STATUS "")
message(STATUS "--- User Options (from cache) ---")
message(STATUS "Cheat system preference: ${NECROSIS_ENABLE_CHEAT_SYSTEM}")
message(STATUS "Debug overlay preference: ${NECROSIS_ENABLE_DEBUG_OVERLAY}")
message(STATUS "Console commands preference: ${NECROSIS_ENABLE_CONSOLE_COMMANDS}")
message(STATUS "")
message(STATUS "=== Effective Feature Configuration ===")
message(STATUS "Cheat system: ${EFFECTIVE_CHEAT_SYSTEM}")
message(STATUS "Debug overlay: ${EFFECTIVE_DEBUG_OVERLAY}")
message(STATUS "Console commands: ${EFFECTIVE_CONSOLE_COMMANDS}")
message(STATUS "Performance metrics: ${EFFECTIVE_PERFORMANCE_METRICS}")
message(STATUS "=====================================")