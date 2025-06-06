#include <iostream>
#include "necrosis/features.h"
#include "necrosis/conditional.h"

// Conditionally include cheat system header only when enabled
#ifdef NECROSIS_CHEAT_SYSTEM_ENABLED
#include "cheat-system/cheat.h"
#endif

int main(int argc, char* argv[])
{
    // KSS -- FIXME -- we'll need to add a command-line parser here
    // to handle command-line arguments and set feature flags.

    // Suppress unused parameter warnings
    (void)argc;
    (void)argv;
    
    std::cout << "\nNecrosis Engine Starting..." << std::endl;

    // Test the conditional compilation system
    std::cout << "\n=== Feature Status ===" << std::endl;
    std::cout << "Cheat System: " << (necrosis::features::cheat_system_enabled ? "ENABLED" : "DISABLED") << std::endl;
    std::cout << "Debug Overlay: " << (necrosis::features::debug_overlay_enabled ? "ENABLED" : "DISABLED") << std::endl;
    std::cout << "Console Commands: " << (necrosis::features::console_commands_enabled ? "ENABLED" : "DISABLED") << std::endl;
    std::cout << "Performance Metrics: " << (necrosis::features::performance_metrics_enabled ? "ENABLED" : "DISABLED") << std::endl;

    // Test conditional execution with actual cheat system integration
    necrosis::conditional<necrosis::features::cheat_system_enabled>::call([]() {
        std::cout << "\n[CHEAT] Initializing cheat system..." << std::endl;

        #ifdef NECROSIS_CHEAT_SYSTEM_ENABLED
        static Necrosis::CheatSystem cheat_system;

        // Register some example cheat commands
        cheat_system.RegisterCheatCommand("god_mode", [](const std::string& args) {
            std::cout << "[CHEAT] God mode activated! Args: " << args << std::endl;
        });

        cheat_system.RegisterCheatCommand("spawn_item", [](const std::string& args) {
            std::cout << "[CHEAT] Spawning item: " << args << std::endl;
        });

        // Test processing commands
        std::cout << "[CHEAT] Testing cheat commands..." << std::endl;
        cheat_system.ProcessCommand("god_mode");
        cheat_system.ProcessCommand("spawn_item sword");
        #endif
    });

    // Test the macro syntax for other features
    NECROSIS_WHEN(debug_overlay)::call([]() {
        std::cout << "[DEBUG] Debug overlay system initialized!" << std::endl;
    });

    NECROSIS_WHEN(console_commands)::call([]() {
        std::cout << "[CONSOLE] Console command system ready!" << std::endl;
    });

    std::cout << "\nEngine initialized successfully." << std::endl;
    return 0;
}