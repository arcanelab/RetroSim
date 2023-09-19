// RetroSim - Copyright 2011-2023 Zoltán Majoros. All rights reserved.
// https://github.com/arcanelab

#include <cstring>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <thread>
#include <algorithm>

#include "GPU.h"
#include "Core.h"
#include "MMU.h"
#include "unscii-8.h"
#include "unscii-16.h"
#include "palette.h"
#include "Logger.h"

#include "SDL.h"

#ifdef TELNET_ENABLED
#include "Telnet/TelnetServer.h"
#endif

using namespace RetroSim::Logger;

namespace RetroSim
{
    RetroSim::Core *RetroSim::Core::instance = nullptr;

    uint32_t frameNumber = 0;

    void InitializeTestPatterns()
    {
        int tileWidth = 8;
        int tileHeight = 16;

        // Generate test map pattern
        int numTiles = (GPU::textureWidth / tileWidth) * (GPU::textureHeight / tileHeight);
        for (int i = 0; i < numTiles; i++)
        {
            MMU::memory.Map_u8[i] = i % 256;
        }

        // load image palette
        MMU::LoadFile(Core::GetInstance()->GetCoreConfig().GetDataPath() + "/freedom.png.pal", MMU::PALETTE_U32);

        // load image bitmap
        MMU::LoadFile(Core::GetInstance()->GetCoreConfig().GetDataPath() + "/freedom.png.bitmap", MMU::BITMAP_U8);

        // copy image from BITMAP_U8 to SPRITE_ATLAS_U8, crop at 128x128
        for (int y = 0; y < 128; y++)
        {
            for (int x = 0; x < 128; x++)
            {
                uint8_t value = MMU::memory.Bitmap_u8[y * 320 + x];
                MMU::memory.SpriteAtlas_u8[y * 128 + x] = value;
            }
        }
    }

    void Core::Initialize(const std::string &basePath)
    {
        // Libretro tends to call initialize multiple times
        if(isInitialized)
            return;

        LogPrintf(RETRO_LOG_INFO, "Initializing RetroSim...\n");
        LogPrintf(RETRO_LOG_INFO, "Base path: %s\n", basePath.c_str());

        // print current directory
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) != nullptr)
            LogPrintf(RETRO_LOG_INFO, "Current working dir: %s\n", cwd);
        else
            LogPrintf(RETRO_LOG_ERROR, "getcwd() error\n");

        coreConfig.Initialize(basePath);
        Reset();

#ifdef TELNET_ENABLED
        std::thread telnetThread(TelnetServer::Start);
        telnetThread.detach();
#endif
        isInitialized = true;
    }

    void Core::InitializePalette()
    {
        // Copy palette to memory
        for (int i = 0; i < 256; i++)
        {
            MMU::memory.Palette_u32[i] = palette_64[i % 64];
        }
    }

    void Core::InitializeFonts()
    {
        size_t length = std::min(unscii_16_length, 0x8000);
        for (int i = 0; i < length; i++)
        {
            MMU::memory.Charset_u8[i] = unscii_16[i];
        }

        length = std::min(unscii_8_length, 0x8000);
        for (int i = 0; i < length; i++)
        {
            MMU::memory.Charset_u8[i+0x8000] = unscii_8[i];
        }

        // copy first 16K from character ram to tile ram
        for (int i = 0; i < 0x4000; i++)
        {
            uint8_t value = MMU::memory.Charset_u8[i];
            MMU::memory.Tiles_u8[i] = value;
        }
    }

    void Core::InitializeCPU()
    {
        // - Set system vectors -
        // The CPU will start executing code at $200
        MMU::WriteMem<uint32_t>(A65000CPU::VEC_RESET, 0x00000200);
        // Stack starts at $1FF and grows downwards
        MMU::WriteMem<uint32_t>(A65000CPU::VEC_STACKPOINTERINIT, 0x000001FF);
        // The CPU will jump here if it encounters an illegal instruction
        MMU::WriteMem<uint32_t>(A65000CPU::VEC_ILLEGALINSTRUCTION, 0x00000200);
        // The interrupt handlers are located at $E000
        MMU::WriteMem<uint32_t>(A65000CPU::VEC_HWIRQ, 0x0000E000);
        MMU::WriteMem<uint32_t>(A65000CPU::VEC_NMI, 0x0000E000);     

        MMU::WriteMem<uint32_t>(0x200, 0x0200180f); // jmp $200

        LoadRetroSimBinaryFile(Core::GetInstance()->GetCoreConfig().GetDataPath() + "/startup.rsb");

        cpu.Reset();
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
        GPU::DrawBitmap(topLeftX, topLeftY, 0, 0, 320, 256, 320, 1);

        GPU::RenderText("This text is 16x16.", textPos, 190, colorIndex);
        GPU::DrawBitmap(topLeftX, topLeftY, 0, 0, 160, 128, 320, 1);        
    }

    int frameCounter = 0;
    float clock = 0;
    uint32_t startTime = SDL_GetTicks(); 

    void Core::RunNextFrame()
    {        
        uint32_t cpuBefore = SDL_GetTicks();
        {
            std::lock_guard<std::mutex> lock(memoryMutex);
            DrawTestScreen();
        }
        
        int cycles = 0;
        while(cycles < 20000)
        {
            cycles += cpu.Tick();
        }
        uint32_t cpuAfter = SDL_GetTicks();
        int timeDelta = cpuAfter - cpuBefore;
        clock += timeDelta;

        uint32_t currentTime = SDL_GetTicks();
        if(currentTime - startTime > 1000.0f)
        {
            startTime = currentTime;
            printf("CPU time: %d ms, fps = %d\n", timeDelta, frameCounter);
            clock = 0;
            frameCounter = 0;
        }

        frameCounter++;
    }

    void Core::Reset()
    {
        std::lock_guard<std::mutex> lock(memoryMutex);

        GPU::Initialize();
        InitializeFonts();
        InitializePalette();
        InitializeTestPatterns();
        InitializeCPU();
    }

    void Core::LoadRetroSimBinaryFile(const std::string &path)
    {
        FILE *file = fopen(path.c_str(), "rb");
        if (file == nullptr)
        {
            LogPrintf(RETRO_LOG_ERROR, "Failed to open file: %s\n", path.c_str());
            return;
        }

        fseek(file, 0, SEEK_END);
        size_t fileSize = ftell(file);
        fseek(file, 0, SEEK_SET);

        uint8_t *buffer = new uint8_t[fileSize];
        fread(buffer, 1, fileSize, file);
        fclose(file);

        uint8_t *ptr = buffer;
        uint8_t magic[3];
        magic[0] = *ptr++;
        magic[1] = *ptr++;
        magic[2] = *ptr++;
        uint8_t version = *ptr++;

        if (magic[0] != 'R' || magic[1] != 'S' || magic[2] != 'X')
        {
            LogPrintf(RETRO_LOG_ERROR, "Invalid magic bytes in file: %s\n", path.c_str());
            return;
        }

        if (version != '0')
        {
            LogPrintf(RETRO_LOG_ERROR, "Invalid version in file: %s\n", path.c_str());
            return;
        }

        while (ptr < buffer + fileSize)
        {
            uint32_t address = *(uint32_t *)ptr;
            ptr += 4;
            uint32_t length = *(uint32_t *)ptr;
            ptr += 4;

            if (address + length > MMU::memorySize)
            {
                LogPrintf(RETRO_LOG_ERROR, "Invalid address in file: %s\n", path.c_str());
                return;
            }

            memcpy(&MMU::memory.raw[address], ptr, length);
            ptr += length;

            LogPrintf(RETRO_LOG_INFO, "Loaded %d bytes to $%x from %s\n", length, address, path.c_str());
        }

        delete[] buffer;
    }

    void Core::Shutdown()
    {
#ifdef TELNET_ENABLED
        TelnetServer::Stop();
#endif
    }
}
