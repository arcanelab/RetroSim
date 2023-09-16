// RetroSim - Copyright 2011-2023 Zoltán Majoros. All rights reserved.
// https://github.com/arcanelab

#pragma once
#include <string>
#include "MMU.h"

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
        stopGPU,
        startGPU,
        stopCPU,
        startCPU,
        reset,
        renderFrame,
        step,
        run,
        quit,
        showInfo,
        showMemoryMap,
        saveScreenshot,
        saveMemoryToFile,
        loadFileToMemory,
    };

    std::string ProcessCommand(const std::string &command);

    std::string DisplayHelp();

    template <typename T>
    std::string SetMemory(std::vector<std::string> tokens);
}
