// RetroSim - Copyright 2011-2023 Zoltán Majoros. All rights reserved.
// https://github.com/arcanelab

#pragma once
#include <cstdint>
#include <stdlib.h>

class MMU
{
    uint8_t *memory;

public:
    MMU(uint32_t sizeInBytes)
    {
        memory = (uint8_t *)malloc(sizeInBytes);
    }

    ~MMU()
    {
        free(memory);
    }

    template <typename T>
    inline T ReadMem(uint32_t address)
    {
        return *((T *)(memory + address));
    }

    template <typename T>
    inline void WriteMem(uint32_t address, T value)
    {
        *(T *)(memory + address) = value;
    }
};
