// RetroSim - Copyright 2011-2023 Zoltán Majoros. All rights reserved.
// https://github.com/arcanelab

#include "MMU.h"
#include "FileUtils.h"

using namespace RetroSim::Logger;

namespace RetroSim::MMU
{
    MemorySections memory;

    int LoadFile(const char *path, uint32_t address)
    {
        const char *filename = ConvertToWindowsPath(std::string(path)).c_str();
        FILE *file = fopen(filename, "rb");
        if (file == nullptr)
        {
            LogPrintf(RETRO_LOG_ERROR, "Failed to open file: %s\n", filename);
            return -1;
        }

        fseek(file, 0, SEEK_END);
        uint32_t fileSize = ftell(file);
        fseek(file, 0, SEEK_SET);

        if (address + fileSize > memorySize)
        {
            LogPrintf(RETRO_LOG_ERROR, "LoadFile: file size exceeds memory size: %08X\n", address + fileSize);
            return -1;
        }

        fread(memory.raw + address, 1, fileSize, file);
        fclose(file);

        LogPrintf(RETRO_LOG_INFO, "Loaded %d bytes to $%x from %s\n", fileSize, address, filename);

        return 0;
    }

    int LoadFile(std::string filename, uint32_t address)
    {
        return LoadFile(ConvertToWindowsPath(filename).c_str(), address);
    }
}
