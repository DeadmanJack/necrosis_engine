#pragma once

#include <string>
#include <functional>
#include <unordered_map>

namespace Necrosis
{
    class CheatSystem
    {
    public:
        using CheatCommand = std::function<void(const std::string&)>;

        void RegisterCheatCommand(const std::string& command, CheatCommand callback);
        void ProcessCommand(const std::string& input);

    private:
        std::unordered_map<std::string, CheatCommand> cheat_commands;
    };
}