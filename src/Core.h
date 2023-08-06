// RetroSim - Copyright 2011-2023 Zoltán Majoros. All rights reserved.
// https://github.com/arcanelab

#pragma once

#include <string>
#include "CoreConfig.h"

namespace RetroSim
{
    class Core
    {
    public:
        static Core *GetInstance()
        {
            if (instance == nullptr)
            {
                instance = new Core();
            }
            return instance;
        }

        void Initialize(const std::string &basePath);
        bool LoadCartridge(const std::string &path);
        CoreConfig GetCoreConfig();
        void RunNextFrame();
        void Reset();

        enum MemoryMap
        {
            TILE_MODE_U8 = 0x100, // valid values = 0..3
            MAP_OFFSET_U32 = 0x101,
            PALETTE_OFFSET_U16 = 0x105,
            PALETTE_MEMORY_U32 = 0x1000, // 256 RGBA elements
            MAP_MEMORY_U8 = 0x2000,
            TILE_MEMORY_U8 = 0x6000, // 256 elements
        };

    private:
        static Core *instance;
        CoreConfig coreConfig;

        void LoadFont();
    };
}