// RetroSim - Copyright 2011-2023 Zoltán Majoros. All rights reserved.
// https://github.com/arcanelab

#pragma once
#include <cstdint>
#include <string>
#include <new> // For placement new

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
        CHARSET_U8 = 0x30000      // Character tile data (64K)
    };

    enum GPURegisters
    {
        TILE_WIDTH = 0xD000,
        TILE_HEIGHT = 0xD001,
        MAP_WIDTH = 0xD002,
        MAP_HEIGHT = 0xD003,
        SPRITE_ATLAS_PITCH = 0xD004,
    };

    struct MemorySections
    {
        // memory sections
        uint32_t *Palette_u32;
        uint8_t *Map_u8;
        uint8_t *Tiles_u8;
        uint8_t *SpriteAtlas_u8;
        uint8_t *Bitmap_u8;
        uint8_t *Charset_u8;
        // registers
        uint8_t *TileWidth_u8;
        uint8_t *TileHeight_8;
        uint8_t *MapWidth_u8;
        uint8_t *MapHeight_u8;
        uint8_t *SpriteAtlasPitch_u8;

        uint8_t raw[memorySize];

        MemorySections()
        {
            Palette_u32 = (uint32_t *)&raw[PALETTE_U32];
            Map_u8 = &raw[MAP_U8];
            Tiles_u8 = &raw[TILES_U8];
            SpriteAtlas_u8 = &raw[SPRITE_ATLAS_U8];
            Bitmap_u8 = &raw[BITMAP_U8];
            Charset_u8 = &raw[CHARSET_U8];
        }
    };

    extern MemorySections memory;

    template <typename T>
    T ReadMem(uint32_t address);

    template <typename T>
    void WriteMem(uint32_t address, T value);

    int LoadFile(const char *filename, uint32_t address);
    int LoadFile(std::string filename, uint32_t address);
}