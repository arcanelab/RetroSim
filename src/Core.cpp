// RetroSim - Copyright 2011-2023 Zoltán Majoros. All rights reserved.
// https://github.com/arcanelab

#include "GPU.h"
#include "Core.h"
#include "MMU.h"
#include <cstring>
#include <stdlib.h>
#include <stdio.h>

namespace RetroSim
{
    RetroSim::Core *RetroSim::Core::instance = nullptr;

    void Core::Initialize(const std::string &basePath)
    {
        coreConfig.Initialize(basePath);
        MMU::WriteMem<uint8_t>(TILE_MODE_U8, GPU::TILE_MODE_8x16);

        LoadFont();

        int tileWidth = 8;
        int tileHeight = 16;

        // Generate palette
        for (int i = 0; i < 256; i++)
        {
            // mmu->WriteMem<uint32_t>(Core::PALETTE_MEMORY_U32 + i * 4, i << 8 | 0xff);
            MMU::WriteMem<uint32_t>(Core::PALETTE_MEMORY_U32 + i * 4, 0xffc0c0c0); // 0xAARRGGBB
        }
        MMU::WriteMem<uint32_t>(Core::PALETTE_MEMORY_U32, 0xff202020); // 0xAARRGGBB = ARGB8888
    }

    void Core::LoadFont()
    {
        // std::string basePathStr(basePath);
        const char *path = (coreConfig.GetDataPath() + "/unscii-16.tiledata").c_str();
        if (MMU::LoadFile(path, Core::TILE_MEMORY_U8) == -1)
        {
            printf("Failed to load file\n");
        }
    }

    CoreConfig Core::GetCoreConfig()
    {
        return coreConfig;
    }

    bool Core::LoadCartridge(const std::string &path)
    {
        return true;
    }

    void Core::RunNextFrame()
    {
        RetroSim::GPU::RenderTileMode();
    }

    void Core::Reset()
    {
    }
}
