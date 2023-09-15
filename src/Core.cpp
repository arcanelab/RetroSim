// RetroSim - Copyright 2011-2023 Zoltán Majoros. All rights reserved.
// https://github.com/arcanelab

#include <cstring>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <thread>
#include "GPU.h"
#include "Core.h"
#include "MMU.h"
#include "unscii-8.h"
#include "unscii-16.h"
#include "palette.h"
#include "Logger.h"
#include "Telnet/TelnetServer.h"

using namespace RetroSim::Logger;

namespace RetroSim
{
    RetroSim::Core *RetroSim::Core::instance = nullptr;

    uint32_t frameNumber = 0;

    void Core::Initialize(const std::string &basePath)
    {
        LogPrintf(RETRO_LOG_INFO, "Initializing RetroSim...\n");
        LogPrintf(RETRO_LOG_INFO, "Base path: %s\n", basePath.c_str());

        // print current directory
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) != nullptr)
            LogPrintf(RETRO_LOG_INFO, "Current working dir: %s\n", cwd);
        else
            LogPrintf(RETRO_LOG_ERROR, "getcwd() error\n");

        coreConfig.Initialize(basePath);
        GPU::Initialize();

        LoadFonts();

        int tileWidth = 8;
        int tileHeight = 16;

        // Copy palette to memory
        for (int i = 0; i < 256; i++)
        {
            MMU::memory.Palette_u32[i] = palette_64[i % 64];
        }

        // Generate test map pattern
        int numTiles = (GPU::textureWidth / tileWidth) * (GPU::textureHeight / tileHeight);
        for (int i = 0; i < numTiles; i++)
        {
            MMU::memory.Map_u8[i] = i % 256;
        }

        // load image palette
        MMU::LoadFile(coreConfig.GetDataPath() + "/freedom.png.pal", MMU::PALETTE_U32);

        // load image bitmap
        MMU::LoadFile(coreConfig.GetDataPath() + "/freedom.png.bitmap", MMU::BITMAP_U8);

        // copy image from BITMAP_U8 to SPRITE_ATLAS_U8, crop at 128x128
        for (int y = 0; y < 128; y++)
        {
            for (int x = 0; x < 128; x++)
            {
                uint8_t value = MMU::memory.Bitmap_u8[y * 320 + x];
                MMU::memory.SpriteAtlas_u8[y * 128 + x] = value;
            }
        }

#ifndef LIBRETRO
        std::thread telnetThread(TelnetServer::Start);
        telnetThread.detach();
#endif
    }

    void Core::LoadFonts()
    {
        for (int i = 0; i < 0x8000; i++)
        {
            MMU::memory.Charset_u8[i] = unscii_16[i];
        }

        uint32_t offset = 0x8000;
        for (int i = 0; i < 0x8000; i++)
        {
            MMU::memory.Charset_u8[i + offset] = unscii_8[i];
        }

        // copy first 16K from character ram to tile ram
        for (int i = 0; i < 0x4000; i++)
        {
            uint8_t value = MMU::memory.Charset_u8[i];
            MMU::memory.Tiles_u8[i] = value;
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
    
    int textPos = 0;

    void DrawTestScreen()
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

        // GPU::ClearScreen(0);
        GPU::RenderOpaqueText("RetroSim", textPos, 150, colorIndex, 20);
        // GPU::Map(frameNumber % GPU::textureWidth, 40, 0, 0, 20, 3, 0);

        textPos--;
        if(textPos < -200)
            textPos = GPU::textureWidth + 200;

        GPU::SetFont(8, 8, 0x8000);
        GPU::RenderText("This text is 8x8.", textPos, 170, colorIndex);
        GPU::SetFont(8, 16, 0);

        // GPU::ClearScreenIgnoreClipping((frameNumber / 30) % 256);

        int bitmapX = 100 + sin(frameNumber / 60.0) * 100;
        int bitmapY = 50 + cos(frameNumber / 60.0) * 100;

        // GPU::DrawBitmap(bitmapX, bitmapY, 0, 0, 320, 256, 320, 1);
        // GPU::DrawSprite(bitmapX, bitmapY, 0, 0, 128, 128, 1);

        int topLeftX = (GPU::textureWidth - 320) / 2;
        int topLeftY = (GPU::textureHeight - 256) / 2;
        // GPU::DrawBitmap(topLeftX, topLeftY, 0, 0, 320, 256, 320, 1);

        GPU::RenderText("This text is 16x16.", textPos, 190, colorIndex);
        GPU::DrawBitmap(topLeftX, topLeftY, 0, 0, 160, 128, 320, 1);        
    }

    void Core::RunNextFrame()
    {
        std::lock_guard<std::mutex> lock(memoryMutex);
        DrawTestScreen();
    }

    void Core::Reset()
    {
    }

    void Core::Shutdown()
    {
#ifndef LIBRETRO
        TelnetServer::Stop();
#endif
    }
}
