// RetroSim - Copyright 2011-2023 Zoltán Majoros. All rights reserved.
// https://github.com/arcanelab

#include "GPU.h"
#include "Core.h"
#include "MMU.h"
#include <cstring>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "unscii-8.h"
#include "unscii-16.h"
#include "palette.h"

namespace RetroSim
{
    RetroSim::Core *RetroSim::Core::instance = nullptr;

    uint32_t frameNumber = 0;

    void Core::Initialize(const std::string &basePath)
    {
        coreConfig.Initialize(basePath);
        GPU::Initialize();

        LoadFonts();

        int tileWidth = 8;
        int tileHeight = 16;

        // Generate palette
        for (int i = 0; i < 256; i++)
        {
            MMU::WriteMem<uint32_t>(MMU::PALETTE_U32 + i * 4, palette_64[i % 64]); // 0xAARRGGBB
        }

        int numTiles = (GPU::textureWidth / tileWidth) * (GPU::textureHeight / tileHeight);
        for (int i = 0; i < numTiles; i++)
        {
            MMU::WriteMem<uint8_t>(MMU::MAP_U8 + i, i % 256);
        }
    }

    void Core::LoadFonts()
    {
        for (int i = 0; i < 0x8000; i++)
        {
            MMU::WriteMem<uint8_t>(MMU::CHARSET + i, unscii_16[i]);
        }

        uint32_t offset = 0x8000;
        for (int i = 0; i < 0x8000; i++)
        {
            MMU::WriteMem<uint8_t>(MMU::CHARSET + i + offset, unscii_8[i]);
        }

        // copy first 16K from character ram to tile ram
        for (int i = 0; i < 0x4000; i++)
        {
            uint8_t value = MMU::ReadMem<uint8_t>(MMU::CHARSET + i);
            MMU::WriteMem<uint8_t>(MMU::TILES_U8 + i, value);
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
        frameNumber++;
        // RetroSim::GPU::RenderTileMode();

        // Test all GPU functions. The color parameter is an index into a 256-long palette.
        // All drawings must be visible on the screen of 480x256 pixels.
        // GPU::Cls();
        GPU::Clip(100, 150, 480 - 100, 256 - 50);
        GPU::Rect(0, 0, 479, 255, 0, true);
        GPU::Line(0, 0, 479, 255, 1);
        GPU::Circle(480 / 2, 256 / 2, 128, 2, false);
        GPU::Circle(240, 128, 50, 3, true);
        GPU::Rect(100, 100, 100, 100, 4, false);
        GPU::NoClip();
        GPU::Rect(100, 100, 50, 50, 5, true);
        GPU::Tri(200, 200, 300, 200, 250, 100, 6, false);
        GPU::Tri(200, 200, 300, 200, 250, 100, 7, true);
        GPU::Tex(0, 0, 100, 0, 0, 100, 0, 0, 100, 0, 0, 100);
        GPU::Pixel(200, 200, 8);

        // GPU::Clip(100, 150, 480 - 150, 256 - 150);
        // GPU::Cls();
        // GPU::NoClip();

        srand(frameNumber / 20);
        int radius = sin(frameNumber / 30.0) * 30 + 50;
        int colorIndex = (frameNumber / 20) % 64;

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
        GPU::Circle(x, y, radius, colorIndex, true);

        // GPU::Cls();
        GPU::Print("RetroSim", (GPU::textureWidth - frameNumber) % GPU::textureWidth, 150, colorIndex, 0, 1);
        // GPU::Map(frameNumber % GPU::textureWidth, 40, 0, 0, 20, 3, 0);

        GPU::SetFont(8, 8, 0x8000);
        GPU::Print("This text is 8x8.", (GPU::textureWidth - frameNumber) % GPU::textureWidth, 170, colorIndex, 0, 1);
        GPU::SetFont(8, 16, 0);
    }

    void Core::Reset()
    {
    }
}
