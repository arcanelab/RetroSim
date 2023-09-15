// RetroSim - Copyright 2011-2023 Zoltán Majoros. All rights reserved.
// https://github.com/arcanelab

#include <cstring>
#include <cstdio>
#include <string>
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

        auto it = commands.find(command);
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
