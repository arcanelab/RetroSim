// RetroSim - Copyright 2011-2023 Zoltán Majoros. All rights reserved.
// https://github.com/arcanelab

#pragma once
#include <string>

namespace RetroSim::RemoteMonitor
{
    struct ServerResponse
    {
        std::string data;
        int length;
    };

    enum Command
    {
        displayHelp,        
        displayMemory,
        setMemoryU8,
        setMemoryU16,
        setMemoryU32,
        disconnect,
    };

    std::string ProcessCommand(const std::string &command);
}
