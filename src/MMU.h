// RetroSim - Copyright 2011-2023 Zoltán Majoros. All rights reserved.
// https://github.com/arcanelab

#pragma once
#include <cstdint>

class MMU
{
    uint8_t *memory;
    const uint_fast32_t memorySize;

public:
    MMU(uint32_t sizeInBytes);
    ~MMU();

    template <typename T>
    inline T ReadMem(uint32_t address);

    template <typename T>
    inline void WriteMem(uint32_t address, T value);

    int LoadFile(const char *filename, uint32_t address);
};
