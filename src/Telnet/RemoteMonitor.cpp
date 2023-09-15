// RetroSim - Copyright 2011-2023 Zoltán Majoros. All rights reserved.
// https://github.com/arcanelab

#include <cstring>
#include <cstdio>
#include <string>
#include <vector>
#include <sstream>
#include "RemoteMonitor.h"
#include "Logger.h"

using namespace std;
using namespace RetroSim::Logger;

namespace RetroSim::RemoteMonitor
{
    std::unordered_map<string, Command> commands = {
        {"help", displayHelp},
        {"mem", displayMemory},
        {"set8", setMemoryU8},
        {"set16", setMemoryU16},
        {"set32", setMemoryU32},
    };

    string DisplayHelp()
    {
        return "help, mem, set8, set16, set32";
    }

    string ProcessCommand(const string &command)
    {
        LogPrintf(RETRO_LOG_INFO, "RemoteMonitor: %s\n", command.c_str());

        std::vector<string> tokens;
        std::istringstream iss(command);
        string token;

        while(iss >> token)
            tokens.push_back(token);

        if(tokens.size() == 0)
            return DisplayHelp();

        auto it = commands.find(tokens[0]);
        if (it == commands.end())
        {
            return DisplayHelp();
        }

        switch(it->second)
        {
            case displayHelp:
                return DisplayHelp();
            case displayMemory:
                return "displayMemory";
            case setMemoryU8:
                return "setMemoryU8";
            case setMemoryU16:
                return "setMemoryU16";
            case setMemoryU32:
                return "setMemoryU32";
        }
    }
}
