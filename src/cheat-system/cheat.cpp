#include "cheat.h"
#include <iostream>
#include <sstream>

namespace Necrosis
{
    void CheatSystem::RegisterCheatCommand(const std::string &command, CheatCommand callback)
    {
        cheat_commands[command] = callback;
        std::cout << "[CHEAT] Registered command: " << command << std::endl;
    }

    void CheatSystem::ProcessCommand(const std::string &input)
    {
        if (input.empty()) {
            return;
        }

        // Parse command and arguments
        std::istringstream iss(input);
        std::string command;
        iss >> command;

        // Get remaining arguments
        std::string args;
        std::getline(iss, args);
        if (!args.empty() && args[0] == ' ') {
            args = args.substr(1); // Remove leading space
        }

        // Find and execute command
        auto it = cheat_commands.find(command);
        if (it != cheat_commands.end()) {
            std::cout << "[CHEAT] Executing: " << command << " with args: '" << args << "'" << std::endl;
            it->second(args);
        } else {
            std::cout << "[CHEAT] Unknown command: " << command << std::endl;
        }
    }
} // namespace Necrosis
