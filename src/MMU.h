// RetroSim - Copyright 2011-2023 Zoltán Majoros. All rights reserved.
// https://github.com/arcanelab

#pragma once
#include <cstdint>
#include <string>
#include "Logger.h"

using namespace RetroSim::Logger;

namespace RetroSim::MMU
{
    const uint_fast32_t memorySize = 0x80000; // 512K;

    enum MemoryMap
    {
        PALETTE_U32 = 0x1000,     // Color palette memory (4K)
        MAP_U8 = 0x2000,          // Map memory (16K)
        TILES_U8 = 0x6000,        // Tile memory bank (16K)
        SPRITE_ATLAS_U8 = 0xA000, // Sprite atlas/memory bank (16K)
        GPU_REGISTERS = 0xD000,   // GPU registers (16 bytes)
        BITMAP_U8 = 0x10000,      // Bitmap memory (120K)
        CHARSET_U8 = 0x30000      // Character tile data (64K)
    };

    struct GPURegisters
    {
        uint8_t tileWdith;
        uint8_t tileHeight;
        uint8_t mapWidth;
        uint8_t mapHeight;
        uint8_t spriteAtlasPitch;
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

        GPURegisters *gpu = reinterpret_cast<GPURegisters *>(&raw[GPU_REGISTERS]);

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
    inline T ReadMem(uint32_t address)
    {
        if (address < memorySize)
        {
            return *((T *)(memory.raw + address));
        }
        else
        {
            LogPrintf(RETRO_LOG_ERROR, "ReadMem: invalid address: %08X\n", address);
            return 0;
        }
    }

    template <typename T>
    inline void WriteMem(uint32_t address, T value)
    {
        if (address < memorySize)
        {
            *(T *)(memory.raw + address) = value;
        }
        else
        {
            LogPrintf(RETRO_LOG_ERROR, "WriteMem: invalid address: %08X\n", address);
        }
    }

    int LoadFile(const char *filename, uint32_t address);
    int LoadFile(std::string filename, uint32_t address);
}