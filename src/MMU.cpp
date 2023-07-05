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
