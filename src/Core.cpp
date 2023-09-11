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
#include "Logger.h"

namespace RetroSim
{
    RetroSim::Core *RetroSim::Core::instance = nullptr;

    uint32_t frameNumber = 0;

    void Core::Initialize(const std::string &basePath)
    {
        Logger::RSPrintf(RETRO_LOG_INFO, "Initializing RetroSim...\n");
        Logger::RSPrintf(RETRO_LOG_INFO, "Base path: %s\n", basePath.c_str());

        // print current directory
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) != nullptr)
            Logger::RSPrintf(RETRO_LOG_INFO, "Current working dir: %s\n", cwd);
        else
            Logger::RSPrintf(RETRO_LOG_ERROR, "getcwd() error\n");

        coreConfig.Initialize(basePath);
        GPU::Initialize();

        LoadFonts();

        int tileWidth = 8;
        int tileHeight = 16;

        // Copy palette to memory
        for (int i = 0; i < 256; i++)
        {
            MMU::WriteMem<uint32_t>(MMU::PALETTE_U32 + i * 4, palette_64[i % 64]); // 0xAARRGGBB
        }

        // Generate test map pattern
        int numTiles = (GPU::textureWidth / tileWidth) * (GPU::textureHeight / tileHeight);
        for (int i = 0; i < numTiles; i++)
        {
            MMU::WriteMem<uint8_t>(MMU::MAP_U8 + i, i % 256);
        }

        // load image palette
        MMU::LoadFile("data/freedom.png.pal", MMU::PALETTE_U32);

        // load image bitmap
        MMU::LoadFile("data/freedom.png.bitmap", MMU::BITMAP_U8);
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
        GPU::SetClipping(100, 150, 480 - 100, 256 - 50);
        GPU::DrawRect(0, 0, 479, 255, 0, true);
        GPU::DrawLine(0, 0, 479, 255, 1);
        GPU::DrawCircle(480 / 2, 256 / 2, 128, 2, false);
        GPU::DrawCircle(240, 128, 50, 3, true);
        // GPU::DrawRect(100, 100, 100, 100, 4, false);
        GPU::DisableClipping();
        // GPU::DrawRect(100, 100, 50, 50, 5, true);
        GPU::DrawTriangle(200, 200, 300, 200, 250, 100, 6, false);
        GPU::DrawTriangle(200, 200, 300, 200, 250, 100, 7, true);
        GPU::DrawTexturedTriangle(0, 0, 100, 0, 0, 100, 0, 0, 100, 0, 0, 100);
        GPU::DrawPixel(200, 200, 8);

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
        GPU::DrawCircle(x, y, radius, colorIndex, true);

        // GPU::Cls();
        GPU::RenderOpaqueText("RetroSim", (GPU::textureWidth - frameNumber) % GPU::textureWidth, 150, colorIndex, 20);
        // GPU::Map(frameNumber % GPU::textureWidth, 40, 0, 0, 20, 3, 0);

        GPU::SetFont(8, 8, 0x8000);
        GPU::RenderText("This text is 8x8.", (GPU::textureWidth - frameNumber) % GPU::textureWidth, 170, colorIndex);
        GPU::SetFont(8, 16, 0);

        GPU::ClearScreenIgnoreClipping((frameNumber / 30) % 256);

        // move bitmap around in a CIRCLE, use sinus
        int bitmapX = 100 + sin(frameNumber / 60.0) * 100;
        int bitmapY = 50 + cos(frameNumber / 60.0) * 100;

        GPU::DrawBitmap(bitmapX, bitmapY, 0, 0, 320, 256, 320, 1);

        int topLeftX = (GPU::textureWidth - 320) / 2;
        int topLeftY = (GPU::textureHeight - 256) / 2;
        GPU::DrawBitmap(topLeftX, topLeftY, 0, 0, 160, 128, 320, 1);
    }

    void Core::Reset()
    {
    }
}
