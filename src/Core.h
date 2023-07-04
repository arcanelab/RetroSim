// RetroSim - Copyright 2011-2023 Zoltán Majoros. All rights reserved.
// https://github.com/arcanelab

#pragma once
#include <cstdint>
#include "MMU.h"

class GPU;

class Core
{
public:
    GPU *gpu;
    MMU *mmu;

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

    const uint_fast32_t MEMSIZE = 0x10000; // 64K

    void Render();
};
