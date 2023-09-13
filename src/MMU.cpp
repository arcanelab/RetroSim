// RetroSim - Copyright 2011-2023 Zoltán Majoros. All rights reserved.
// https://github.com/arcanelab

#include "MMU.h"
#include "Logger.h"
#include <stdlib.h>
#include <stdio.h>

using namespace RetroSim::Logger;

namespace RetroSim::MMU
{
    // Explicit instantiation
    template uint8_t ReadMem<uint8_t>(uint32_t address);
    template uint16_t ReadMem<uint16_t>(uint32_t address);
    template uint32_t ReadMem<uint32_t>(uint32_t address);

    template void WriteMem<uint8_t>(uint32_t address, uint8_t value);
    template void WriteMem<uint16_t>(uint32_t address, uint16_t value);
    template void WriteMem<uint32_t>(uint32_t address, uint32_t value);

    MemorySections memory;

    template <typename T>
    inline T ReadMem(uint32_t address)
    {
        if (address < memorySize)
        {
            return *((T *)(memory.raw + address));
        }
        else
        {
            LogPrintf(RETRO_LOG_ERROR, "ReadMem: invalid address: %08X\n", address);
            return 0;
        }
    }

    template <typename T>
    inline void WriteMem(uint32_t address, T value)
    {
        if (address < memorySize)
        {
            *(T *)(memory.raw + address) = value;
        }
        else
        {
            LogPrintf(RETRO_LOG_ERROR, "WriteMem: invalid address: %08X\n", address);
        }
    }

    int LoadFile(const char *filename, uint32_t address)
    {
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
}
