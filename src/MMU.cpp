// RetroSim - Copyright 2011-2023 Zoltán Majoros. All rights reserved.
// https://github.com/arcanelab

#include <cstdint>
#include <stdlib.h>
#include <stdio.h>
#include "MMU.h"

template uint8_t MMU::ReadMem<uint8_t>(uint32_t address);
template uint16_t MMU::ReadMem<uint16_t>(uint32_t address);
template uint32_t MMU::ReadMem<uint32_t>(uint32_t address);

template void MMU::WriteMem<uint8_t>(uint32_t address, uint8_t value);
template void MMU::WriteMem<uint16_t>(uint32_t address, uint16_t value);
template void MMU::WriteMem<uint32_t>(uint32_t address, uint32_t value);

MMU::MMU(uint32_t sizeInBytes) : memorySize(sizeInBytes)
{
    memory = (uint8_t *)malloc(sizeInBytes);
}

MMU::~MMU()
{
    free(memory);
}

template <typename T>
inline T MMU::ReadMem(uint32_t address)
{
    if (address < memorySize)
    {
        return *((T *)(memory + address));
    }
    else
    {
        printf("ReadMem: invalid address: %08X\n", address);
        return 0;
    }
}

template <typename T>
inline void MMU::WriteMem(uint32_t address, T value)
{
    if (address < memorySize)
    {
        *(T *)(memory + address) = value;
    }
    else
    {
        printf("WriteMem: invalid address: %08X\n", address);
    }
}

int MMU::LoadFile(const char *filename, uint32_t address)
{
    FILE *file = fopen(filename, "rb");
    if (file == nullptr)
    {
        printf("Failed to open file: %s\n", filename);
        return -1;
    }

    fseek(file, 0, SEEK_END);
    uint32_t fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (address + fileSize > memorySize)
    {
        printf("LoadFile: file size exceeds memory size: %08X\n", address + fileSize);
        return -1;
    }

    fread(memory + address, 1, fileSize, file);
    fclose(file);

    return 0;
}
