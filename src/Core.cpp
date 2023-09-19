// RetroSim - Copyright 2011-2023 Zoltán Majoros. All rights reserved.
// https://github.com/arcanelab

#include <cstring>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <thread>
#include <algorithm>
#include <chrono>

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

        LogPrintf(RETRO_LOG_INFO, "Copied $%x bytes from unscii_16 to $%x-$%x\n", length, MMU::CHARSET_U8, MMU::CHARSET_U8 + length);
        LogPrintf(RETRO_LOG_INFO, "Copied $%x bytes from unscii_8 to $%x-$%x\n", length, MMU::CHARSET_U8 + 0x8000, MMU::CHARSET_U8 + 0x8000 + length);
        LogPrintf(RETRO_LOG_INFO, "Copied $%x bytes from $%x to $%x-$%x\n", 0x4000, MMU::CHARSET_U8, MMU::TILES_U8, MMU::TILES_U8 + 0x4000);
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

        MMU::memory.generalRegisters.refreshRate = coreConfig.GetFPS();
        MMU::memory.generalRegisters.fixedFrameTime = 1000000 / coreConfig.GetFPS(); // microseconds

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
        GPU::ClearScreenIgnoreClipping();
        int colorIndex = (frameNumber / 20) % 64;

        textPos--;
        if(textPos < -200)
            textPos = GPU::textureWidth + 200;

        GPU::RenderOpaqueText("RetroSim", textPos, 150, colorIndex, 20);

        int bitmapX = 100 + sin(frameNumber / 100.0) * 100;
        int bitmapY = 50 + cos(frameNumber / 100.0) * 100;

        GPU::DrawBitmap(bitmapX, bitmapY, 0, 0, 320, 256, 320, 1);

        GPU::SetFont(8, 8, 0x8000);
        GPU::RenderText("This text is 8x8.", textPos, 170, colorIndex);
        GPU::SetFont(8, 16, 0);
    }

    float clock = 0;
    uint32_t cpuStartTime = SDL_GetTicks();
    auto lastFrameTime = std::chrono::high_resolution_clock::now();

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
        if(currentTime - cpuStartTime > 1000.0f)
        {
            cpuStartTime = currentTime;
            printf("CPU time: %d ms, fps = %d, deltaTime = %d, currentFPS = %d\n", timeDelta, frameCounter, MMU::memory.generalRegisters.deltaTime, MMU::memory.generalRegisters.currentFPS);
            MMU::memory.generalRegisters.currentFPS = frameCounter;
            clock = 0;
            frameCounter = 0;
        }

        MMU::memory.generalRegisters.frameCounter = frameCounter;
        frameCounter++;

        auto now = std::chrono::high_resolution_clock::now();
        auto frameTimeInMicroseconds = std::chrono::duration_cast<std::chrono::microseconds>(now - lastFrameTime).count();
        MMU::memory.generalRegisters.deltaTime = frameTimeInMicroseconds;
        MMU::memory.generalRegisters.currentFPS = 1000000.0f / frameTimeInMicroseconds;
        lastFrameTime = now;
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
