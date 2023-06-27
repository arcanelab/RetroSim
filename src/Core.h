// RetroSim - Copyright 2011-2023 Zoltán Majoros. All rights reserved.
// https://github.com/arcanelab

#pragma once
#include <cstdint>

class GPU;

class Core
{
private:
    GPU *gpu;
    uint8_t *memory;

public:
    enum MemoryMap
    {
        TILE_MODE_U8 = 0x100, // valid values = 0..3
        MAP_OFFSET_U32 = 0x101,
        PALETTE_OFFSET_U16 = 0x105,
        PALETTE_MEMORY_U32 = 0x1000, // 256 RGBA elements
        MAP_MEMORY_U8 = 0x2000,
        TILE_MEMORY_U8 = 0x6000, // 256 elements
    };

    Core();
    ~Core(){};

    const uint_fast32_t MEMSIZE = 0x10000; // 64KB

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
