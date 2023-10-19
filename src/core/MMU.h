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
        MAP_U8 = 0x1000,            // Map memory (16K)
        TILES_U8 = 0x5000,          // Tile memory bank (16K)
        SPRITE_ATLAS_U8 = 0x9000,   // Sprite atlas/memory bank (16K)
        GPU_REGISTERS = 0xD000,     // GPU registers
        GENERAL_REGISTERS = 0xD100, // General registers
        PALETTE_U32 = 0xE000,       // Color palette memory (4K)
        BITMAP_U8 = 0x10000,        // Bitmap memory (120K)
        CHARSET_U8 = 0x30000        // Character tile data (64K)
    };

    struct GPURegisters
    {
        uint16_t screenWidth;
        uint16_t screenHeight;
        uint8_t tileWidth;
        uint8_t tileHeight;
        uint8_t mapWidth;
        uint8_t mapHeight;
        uint8_t spriteAtlasPitch;
    };

    struct GeneralRegisters
    {
        uint32_t fixedFrameTime; // in microseconds (µs)
        uint32_t deltaTime;      // in microseconds (µs)
        uint32_t frameCounter;   // number of frames since the start of the program
        uint8_t refreshRate;     // in Hz
        uint8_t currentFPS;      // in Hz
    };

    struct MemorySections
    {
        uint8_t raw[memorySize];

        // memory sections
        uint8_t *Map_u8;
        uint8_t *Tiles_u8;
        uint8_t *SpriteAtlas_u8;
        uint32_t *Palette_u32;
        uint8_t *Bitmap_u8;
        uint8_t *Charset_u8;

        GPURegisters &gpu;
        GeneralRegisters &generalRegisters;

        MemorySections()
            : gpu(*reinterpret_cast<GPURegisters *>(&raw[GPU_REGISTERS])), // Initializing references in the constructor's initialization list
              generalRegisters(*reinterpret_cast<GeneralRegisters *>(&raw[GENERAL_REGISTERS]))
        {
            memset(raw, 0, memorySize);
            Map_u8 = &raw[MAP_U8];
            Tiles_u8 = &raw[TILES_U8];
            SpriteAtlas_u8 = &raw[SPRITE_ATLAS_U8];
            Palette_u32 = (uint32_t *)&raw[PALETTE_U32];
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
        if (address < (memorySize - sizeof(T)))
        {
            *(T *)(memory.raw + address) = value;
        }
        else
        {
            LogPrintf(RETRO_LOG_ERROR, "WriteMem: invalid address: $%08X. Upper bound: $%08X.\n", address, memorySize);
        }
    }

    int LoadFile(const char *filename, uint32_t address);
    int LoadFile(std::string filename, uint32_t address);
}