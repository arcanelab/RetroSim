// RetroSim - Copyright 2011-2023 Zoltán Majoros. All rights reserved.
// https://github.com/arcanelab

#include "GPU.h"
#include "Core.h"
#include "MMU.h"
#include <cstring>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

namespace RetroSim
{
    RetroSim::Core *RetroSim::Core::instance = nullptr;

    void Core::Initialize(const std::string &basePath)
    {
        coreConfig.Initialize(basePath);

        LoadFont();

        int tileWidth = 8;
        int tileHeight = 16;

        // Generate palette
        for (int i = 0; i < 256; i++)
        {
            // mmu->WriteMem<uint32_t>(Core::PALETTE_MEMORY_U32 + i * 4, i << 8 | 0xff);
            MMU::WriteMem<uint32_t>(MMU::PALETTE_U32 + i * 4, 0xffc0c0c0); // 0xAARRGGBB
        }
        MMU::WriteMem<uint32_t>(MMU::PALETTE_U32, 0xff202020); // 0xAARRGGBB = ARGB8888
    }

    void Core::LoadFont()
    {
        // print current directory
        char basePath[1024];
        getcwd(basePath, 1024);
        printf("Current directory: %s\n", basePath);
        
        std::string finalPath = coreConfig.GetDataPath() + "/unscii-16.tiledata";
        MMU::LoadFile(finalPath.c_str(), MMU::CHARSET);
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
        GPU::frameNumber++;
        // RetroSim::GPU::RenderTileMode();

        // Test all GPU functions. The color parameter is an index into a 256-long palette.
        // All drawings must be visible on the screen of 480x256 pixels.
        // GPU::Cls();
        // GPU::Clip(50, 50, 480-50, 256-50);
        // GPU::Rect(0, 0, 479, 255, 0, true);
        // GPU::Line(0, 0, 479, 255, 1);
        // GPU::Circle(480/2, 256/2, 128, 2, false);
        // GPU::Circle(240, 128, 50, 3, true);
        // GPU::Rect(100, 100, 100, 100, 4, false);
        // GPU::NoClip();
        // GPU::Rect(100, 100, 50, 50, 5, true);
        // GPU::Tri(200, 200, 300, 200, 250, 100, 6, false);
        // GPU::Tri(200, 200, 300, 200, 250, 100, 7, true);
        // GPU::Tex(0, 0, 100, 0, 0, 100, 0, 0, 100, 0, 0, 100);
        // GPU::Pixel(200, 200, 8);
        
        GPU::Clip(100, 100, 480-100, 256-50);
        GPU::Cls();
        GPU::NoClip();

        srand(GPU::frameNumber / 20);
        int radius = rand() % 40 + 10;

        static int x = 0;
        static int y = 0;
        static int dx = 1;
        static int dy = 1;
        x += dx;
        y += dy;
        if (x < 0 || x > 480)
            dx = -dx;
        if (y < 0 || y > 256)
            dy = -dy;
        GPU::Circle(x, y, radius, 2, true);
        
        // change circle color by modifying the palette color
        srand(GPU::frameNumber / 30);
        uint8_t randomR = rand() % 256;
        uint8_t randomG = rand() % 256;
        uint8_t randomB = rand() % 256;

        uint32_t randomColor = randomR << 16 | randomG << 8 | randomB;
        MMU::WriteMem<uint32_t>(MMU::PALETTE_U32 + 2 * 4, randomColor);

        // GPU::Cls();
        GPU::Print("RetroSim", GPU::frameNumber % GPU::textureWidth, 150, 2, 0, 1);
    }

    void Core::Reset()
    {
    }
}
