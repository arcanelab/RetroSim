// RetroSim - Copyright 2011-2023 Zoltán Majoros. All rights reserved.
// https://github.com/arcanelab

#pragma once
#include <cstdint>

namespace RetroSim::MMU
{
    const uint_fast32_t memorySize = 0x80000; // 512K;

    enum MemoryMap
    {
        PALETTE_U32 = 0x1000,  // Color palette memory (4K)
        MAP_U8 = 0x2000,       // Map memory (16K)
        TILES_U8 = 0x6000,     // Tile memory bank (16K)
        SPRITE_ATLAS_U8 = 0xA000, // Sprite atlas/memory bank (16K)
        BITMAP_U8 = 0x10000,      // Bitmap memory (120K)
        CHARSET = 0x30000      // Character tile data (64K)
    };

    enum GPURegisters
    {
        TILE_WIDTH = 0xD000,
        TILE_HEIGHT = 0xD001,
        MAP_WIDTH = 0xD002,
        MAP_HEIGHT = 0xD003,
        PALETTE_BANK = 0xD004,
        MAP_BANK = 0xD005,
        TILE_BANK = 0xD006,
        SPRITE_BANK = 0xD007
    };

    template <typename T>
    T ReadMem(uint32_t address);

    template <typename T>
    void WriteMem(uint32_t address, T value);

    int LoadFile(const char *filename, uint32_t address);
}