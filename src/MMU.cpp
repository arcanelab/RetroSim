// RetroSim - Copyright 2011-2023 Zoltán Majoros. All rights reserved.
// https://github.com/arcanelab

#include "MMU.h"
#include "Logger.h"
#include <stdlib.h>
#include <stdio.h>

namespace RetroSim::MMU
{
    template uint8_t ReadMem<uint8_t>(uint32_t address);
    template uint16_t ReadMem<uint16_t>(uint32_t address);
    template uint32_t ReadMem<uint32_t>(uint32_t address);

    template void WriteMem<uint8_t>(uint32_t address, uint8_t value);
    template void WriteMem<uint16_t>(uint32_t address, uint16_t value);
    template void WriteMem<uint32_t>(uint32_t address, uint32_t value);

    uint8_t *memory = (uint8_t *)calloc(memorySize, 1);

    template <typename T>
    inline T ReadMem(uint32_t address)
    {
        if (address < memorySize)
        {
            return *((T *)(memory + address));
        }
        else
        {
            Logger::RSPrintf(RETRO_LOG_ERROR, "ReadMem: invalid address: %08X\n", address);
            return 0;
        }
    }

    template <typename T>
    inline void WriteMem(uint32_t address, T value)
    {
        if (address < memorySize)
        {
            *(T *)(memory + address) = value;
        }
        else
        {
            Logger::RSPrintf(RETRO_LOG_ERROR, "WriteMem: invalid address: %08X\n", address);
        }
    }

    int LoadFile(const char *filename, uint32_t address)
    {
        FILE *file = fopen(filename, "rb");
        if (file == nullptr)
        {
            Logger::RSPrintf(RETRO_LOG_ERROR, "Failed to open file: %s\n", filename);
            return -1;
        }

        fseek(file, 0, SEEK_END);
        uint32_t fileSize = ftell(file);
        fseek(file, 0, SEEK_SET);

        if (address + fileSize > memorySize)
        {
            Logger::RSPrintf(RETRO_LOG_ERROR, "LoadFile: file size exceeds memory size: %08X\n", address + fileSize);
            return -1;
        }

        fread(memory + address, 1, fileSize, file);
        fclose(file);

        Logger::RSPrintf(RETRO_LOG_INFO, "Loaded %d bytes to $%x from %s\n", fileSize, address, filename);

        return 0;
    }
}