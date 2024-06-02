// RetroSim - Copyright 2011-2023 Zoltán Majoros. All rights reserved.
// https://github.com/arcanelab

#include "MMU.h"
#include "FileUtils.h"

using namespace RetroSim::Logger;

namespace RetroSim::MMU
{
    MemorySections memory;

    int LoadFileToAddress(const std::string& path, uint32_t address)
    {
        size_t fileSize;
        uint8_t *buffer = RetroSim::ReadBinaryFile(path, fileSize);
        if (buffer == nullptr)
        {
            LogPrintf(RETRO_LOG_ERROR, "Failed to open file: %s\n", path.c_str());
            return -1;
        }

        if (address + fileSize > memorySize)
        {
            LogPrintf(RETRO_LOG_ERROR, "LoadFile: file size exceeds memory size: %08X\n", address + fileSize);
            // TODO: free memory or invert condition
            return -1;
        }
        
        memcpy(memory.raw + address, buffer, fileSize);
        delete buffer;

        return 0;
    }
}
