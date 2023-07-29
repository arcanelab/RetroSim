// RetroSim - Copyright 2011-2023 Zoltán Majoros. All rights reserved.
// https://github.com/arcanelab

#pragma once
#include <cstdint>

namespace RetroSim::MMU
{
    const uint_fast32_t memorySize = 0x40000; // 256K;

    template <typename T>
    inline T ReadMem(uint32_t address);

    template <typename T>
    inline void WriteMem(uint32_t address, T value);

    int LoadFile(const char *filename, uint32_t address);
};
