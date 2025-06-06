#pragma once

// Necrosis Engine Feature Configuration
// This file works with preprocessor definitions set by CMake via add_compile_definitions()
// The CMake file sets: NECROSIS_CHEAT_SYSTEM_ENABLED=ON/OFF etc.

// Convert CMake preprocessor definitions to constexpr booleans for template usage
namespace necrosis {
namespace features {

// Cheat system feature
#ifdef NECROSIS_CHEAT_SYSTEM_ENABLED
    constexpr bool cheat_system_enabled = true;
#else
    constexpr bool cheat_system_enabled = false;
#endif

// Debug overlay feature
#ifdef NECROSIS_DEBUG_OVERLAY_ENABLED
    constexpr bool debug_overlay_enabled = true;
#else
    constexpr bool debug_overlay_enabled = false;
#endif

// Console commands feature
#ifdef NECROSIS_CONSOLE_COMMANDS_ENABLED
    constexpr bool console_commands_enabled = true;
#else
    constexpr bool console_commands_enabled = false;
#endif

// Performance metrics feature
#ifdef NECROSIS_PERFORMANCE_METRICS_ENABLED
    constexpr bool performance_metrics_enabled = true;
#else
    constexpr bool performance_metrics_enabled = false;
#endif

} // namespace features
} // namespace necrosis
