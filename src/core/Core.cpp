// RetroSim - Copyright 2011-2023 Zoltán Majoros. All rights reserved.
// https://github.com/arcanelab

#include <cstring>
#include <stdlib.h>
#include <stdio.h>
#ifndef WIN32
#include <unistd.h>
#else
#include <direct.h>
#endif
#include <thread>
#include <algorithm>
#include <chrono>
#include <iostream>
#include <sstream>
#include <fstream>
#include <random>

#ifdef IMGUI
#include "imgui.h"
#include "imfilebrowser.h"
#endif // IMGUI

#include "GravityScripting.h"
#include "GPU.h"
#include "Core.h"
#include "MMU.h"
#include "unscii-8.h"
#include "unscii-16.h"
#include "palette.h"
#include "Logger.h"
#include "A65000Disassembler.h"
#include "Asm65k.h"
#include "Audio.h"
#include "FileUtils.h"

#ifdef SDL
#include "SDL.h"
#endif

#ifdef TELNET_ENABLED
#include "TelnetServer.h"
#endif

using namespace RetroSim::Logger;

namespace RetroSim
{
    RetroSim::Core *RetroSim::Core::instance = nullptr;

    uint32_t frameNumber = 0;

    int GetRandomNumber(int n, int m)
    {
        static std::default_random_engine generator(std::chrono::system_clock::now().time_since_epoch().count());
        std::uniform_int_distribution<int> distribution(n, m);
        return distribution(generator);
    }

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

        std::vector<string> images = {"freedom", "Fairlight", "Metaverse", "Nahkolor", "Rebels", "WinterPatrol", "Phenomenon", "Explorers"};
        int randomIndex = GetRandomNumber(0, (images.size() - 1));

        LogPrintf(RETRO_LOG_INFO, "RandomIndex: %d\n", randomIndex);
        string path = Core::GetInstance()->GetCoreConfig().GetDataPath() + "/gfx/" + images[randomIndex] + ".png.pal";
        LogPrintf(RETRO_LOG_INFO, "Random image: %s\n", path.c_str());

        // load image palette
        MMU::LoadFileToAddress(Core::GetInstance()->GetCoreConfig().GetDataPath() + "/gfx/" + images[randomIndex] + ".png.pal", MMU::PALETTE_U32);

        // load image bitmap
        MMU::LoadFileToAddress(Core::GetInstance()->GetCoreConfig().GetDataPath() + "/gfx/" + images[randomIndex] + ".png.bitmap", MMU::BITMAP_U8);

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

    using namespace std;

    void AssembleStartup()
    {
        // ifstream fs(Core::GetInstance()->GetCoreConfig().GetDataPath() + "/startup.s");
        // stringstream buffer;
        // buffer << fs.rdbuf();
        // fs.close();

        // if (buffer.str().empty())
        // {
        //     printf("Could not load file '%s'\n", "startup.s");
        //     return;
        // }

        string buffer = ReadTextFile(Core::GetInstance()->GetCoreConfig().GetDataPath() + "/startup.s");
        if (buffer.empty())
        {
            printf("Could not load file '%s'\n", "startup.s");
            return;
        }

        stringstream bufferStringStream(buffer);

        AsmA65k asm65k;
        std::vector<Segment> *segments;
        try
        {
            segments = asm65k.assemble(bufferStringStream);
        }
        catch (AsmError error)
        {
            LogPrintf(RETRO_LOG_ERROR, "Assembly error in line %d: \"%s\"\n", error.lineNumber, error.errorMessage.c_str());
            LogPrintf(RETRO_LOG_ERROR, "in line: %s\n", error.lineContent.c_str());
            return;
        }

        for (int i = 0; i < segments->size(); i++)
        {
            Segment actSegment = (*segments)[i];
            uint32_t address = actSegment.address;
            uint32_t length = (uint32_t)actSegment.data.size();

            for (uint32_t memoryPtr = address; memoryPtr < address + length; memoryPtr++)
            {
                // MMU::memory.raw[memoryPtr] = actSegment.data[memoryPtr - address];
                MMU::WriteMem<uint8_t>(memoryPtr, actSegment.data[memoryPtr - address]);
            }
        }
    }

    void Core::Initialize(const std::string &basePath)
    {
        // Libretro tends to call initialize multiple times
        if (isInitialized)
            return;

        LogPrintf(RETRO_LOG_INFO, "Initializing RetroSim...\n");
        LogPrintf(RETRO_LOG_INFO, "Base path: %s\n", basePath.c_str());

#ifdef UNIX_HOST
        // print current directory
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) != nullptr)
        {
            LogPrintf(RETRO_LOG_INFO, "Current working dir: %s\n", cwd);
        }
        else
            LogPrintf(RETRO_LOG_ERROR, "getcwd() error\n");
#endif

        coreConfig.Initialize(basePath);

        Audio::Initialize();

        scriptingEnabled = !coreConfig.GetScriptPath().empty();
        if (scriptingEnabled)
        {
            LogPrintf(RETRO_LOG_INFO, "Running script: %s\n", coreConfig.GetScriptPath().c_str());
            GravityScripting::Initialize();
            GravityScripting::RegisterAPIFunctions();
            GravityScripting::CompileScriptFromFile(coreConfig.GetScriptPath());
            GravityScripting::RunScript("start", {}, 0);
        }

        cpu.syscallHandler = SyscallHandler;
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
            MMU::memory.Charset_u8[i + 0x8000] = unscii_8[i];
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

        // LoadRetroSimBinaryFile(Core::GetInstance()->GetCoreConfig().GetDataPath() + "/startup.rsb");
        // LoadRetroSimBinaryFile(Core::GetInstance()->GetCoreConfig().GetDataPath() + "/test.rsb");
        AssembleStartup();

        A65000Disassembler disasm;
        auto result = disasm.getDisassembly(MMU::memory.raw + 0x200, 0x200, 10);
        for (auto &line : result.text)
        {
            LogPrintf(RETRO_LOG_INFO, "%s\n", line.c_str());
        }

        cpu.Reset();
    }

    void Core::SetRefreshRate(int refreshRate)
    {
        coreConfig.SetTargetFPS(refreshRate);
        MMU::memory.generalRegisters.refreshRate = refreshRate;
        MMU::memory.generalRegisters.fixedFrameTime = 1000000 / refreshRate; // microseconds
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
        if (textPos < -200)
            textPos = GPU::textureWidth + 200;

        GPU::RenderOpaqueText("RetroSim", textPos, 150, colorIndex, 20);

        int bitmapX = (int)(100 + sin(frameNumber / 100.0) * 100);
        int bitmapY = (int)(50 + cos(frameNumber / 100.0) * 100);

        GPU::DrawBitmap(bitmapX, bitmapY, 0, 0, 320, 256, 320, 1);

        GPU::SetFont(8, 8, 0x8000);
        GPU::RenderText("This text is 8x8.", textPos, 170, colorIndex);
        GPU::SetFont(8, 16, 0);
    }

    float clock = 0;

    uint32_t cpuStartTime = 0;
    auto lastFrameTime = std::chrono::high_resolution_clock::now();

    uint32_t GetTicks()
    {
#ifdef SDL
        return SDL_GetTicks();
#else
        return 0;
#endif
    }

    void Core::RunNextFrame()
    {
        uint32_t cpuBefore = GetTicks();
        {
            if (scriptingEnabled)
                GravityScripting::RunScript("update", {}, 0);

            // std::lock_guard<std::mutex> lock(memoryMutex);
            // DrawTestScreen();
        }

        int cycles = 0;

        if (cpu.sleep == false)
        {
            while (cycles < coreConfig.cpuCyclesPerFrame)
            {
                cycles += cpu.Tick();
            }
        }
        uint32_t cpuAfter = GetTicks();
        int timeDelta = cpuAfter - cpuBefore;
        clock += timeDelta;

        uint32_t currentTime = GetTicks();
        if (currentTime - cpuStartTime > 1000.0f)
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
        MMU::memory.generalRegisters.deltaTime = (uint32_t)frameTimeInMicroseconds;
        MMU::memory.generalRegisters.currentFPS = (uint8_t)(1000000.0f / frameTimeInMicroseconds);
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

        isPaused = false;
        frameCounter = 0;
    }

    void Core::LoadRetroSimBinaryFile(const std::string &path)
    {
        FILE *file = fopen(path.c_str(), "rb");
        if (file == nullptr)
        {
            LogPrintf(RETRO_LOG_ERROR, "Failed to systemWindowEnabled file: %s\n", path.c_str());
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

    void Core::SyscallHandler(uint16_t syscallID, uint32_t argumentAddress)
    {
        LogPrintf(RETRO_LOG_DEBUG, "Syscall %d, argument struct address: %8x\n", syscallID, argumentAddress);
    }

    void Core::RenderAudio(uint16_t **audioBuffer, uint32_t *audioBufferSize)
    {
        Audio::RenderAudio();
        *audioBuffer = Audio::GetAudioBuffer();
        *audioBufferSize = Audio::GetAudioBufferSize();
    }

    uint32_t Core::GetSampleRate()
    {
        return Audio::GetSampleRate();
    }

    void Core::Pause()
    {
        isPaused = true;
    }

    void Core::Resume()
    {
        isPaused = false;
    }

    bool Core::IsPaused()
    {
        return isPaused;
    }

#ifdef IMGUI
    void Core::DrawImGui()
    {
        bool systemWindowEnabled = true;
        ImGui::Begin("System", &systemWindowEnabled);
        if (ImGui::Button("Reset system"))
        {
            Reset();
        }

        ImGui::SameLine();

        if (ImGui::Button("Quit system"))
        {
            std::exit(0);
        }

        ImGui::Checkbox("Paused", &isPaused);
        ImGui::Checkbox("Scripting Enabled", &scriptingEnabled);

        if (ImGui::Button("Reset CPU"))
        {
            cpu.Reset();
        }

        if (isPaused)
        {
            if (ImGui::Button("Run next frame"))
            {
                RunNextFrame();
            }

            if (ImGui::Button("Step CPU"))
            {
                cpu.sleep = false;
                cpu.Tick();
            }
        }

        ImGui::End();

        bool infoWindowEnabled = true;
        ImGui::Begin("Info", &systemWindowEnabled);
        ImGuiTreeNodeFlags openHeader = ImGuiTreeNodeFlags_DefaultOpen;

        if (ImGui::CollapsingHeader("System", openHeader))
        {
            ImGui::Text("Fixed Frame Time: %u µs", MMU::memory.generalRegisters.fixedFrameTime);
            ImGui::Text("Delta Time: %u µs", MMU::memory.generalRegisters.deltaTime);
            ImGui::Text("Frame Counter: %u", MMU::memory.generalRegisters.frameCounter);
            ImGui::Text("Refresh Rate: %d Hz", MMU::memory.generalRegisters.refreshRate);
            ImGui::Text("Current FPS: %d Hz", MMU::memory.generalRegisters.currentFPS);
        }

        if (ImGui::CollapsingHeader("GPU", openHeader))
        {
            ImGui::Text("Screen Width: %d", MMU::memory.gpu.screenWidth);
            ImGui::Text("Screen Height: %d", MMU::memory.gpu.screenHeight);
            ImGui::Text("Tile Width: %d", MMU::memory.gpu.tileWidth);
            ImGui::Text("Tile Height: %d", MMU::memory.gpu.tileHeight);
            ImGui::Text("Map Width: %d", MMU::memory.gpu.mapWidth);
            ImGui::Text("Map Height: %d", MMU::memory.gpu.mapHeight);
            ImGui::Text("Sprite Atlas Pitch: %d", MMU::memory.gpu.spriteAtlasPitch);
        }

        if (ImGui::CollapsingHeader("A65000 CPU", openHeader))
        {
            ImGui::Checkbox("Sleep", &cpu.sleep);
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "General registers:");

            const int num_columns = 4;
            if (ImGui::BeginTable("Registers", num_columns))
            {
                for (int i = 0; i < 16; i++)
                {
                    if (i % 4 == 0)
                    {
                        ImGui::TableNextRow();
                    }

                    ImGui::TableSetColumnIndex(i % 4);

                    if (i < 14)
                        ImGui::Text("R%d: %X", i, cpu.registers[i]);
                    else if (i == 14)
                        ImGui::Text("SP: %X", cpu.registers[14]);
                    else
                        ImGui::Text("PC: %X", cpu.registers[15]);
                }

                ImGui::EndTable();
            }

            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Status register:");
            ImGui::Text("Z: %X  N: %X  C: %X  V: %X  B: %X  I: %X",
                        cpu.statusRegister.z, cpu.statusRegister.n, cpu.statusRegister.c,
                        cpu.statusRegister.v, cpu.statusRegister.b, cpu.statusRegister.i);
        }

        ImGui::End();

        bool configWindowEnabled;
        ImGui::Begin("Config", &configWindowEnabled);

        ImGui::Text("Data Path: %s", coreConfig.GetDataPath().c_str());
        ImGui::Text("Script Path: %s", coreConfig.GetScriptPath().c_str());
        ImGui::Text("Fullscreen: %s", coreConfig.IsFullScreen() ? "True" : "False");
        ImGui::Text("FPS Override: %d", coreConfig.GetFPS());
        ImGui::Text("Audio Sample Rate: %d Hz", coreConfig.GetAudioSampleRate());
        ImGui::Text("Window Scale: %d", coreConfig.GetWindowScale());
        ImGui::InputInt("CPU cycles per frame", &coreConfig.cpuCyclesPerFrame);

        ImGui::End();

        bool scriptsWindowEnabled;
        ImGui::Begin("Scripts", &scriptsWindowEnabled);

        static ImGui::FileBrowser fileDialog(0, "..");
        
        // (optional) set browser properties
        fileDialog.SetTitle("Select a script file");
        fileDialog.SetTypeFilters({ ".gravity" });

        // open file dialog when user clicks this button
        if(ImGui::Button("Load script"))
            fileDialog.Open();
        
        fileDialog.Display();
        
        if(fileDialog.HasSelected())
        {
            auto fileName = fileDialog.GetSelected().string();
            std::cout << "Selected filename" << fileName << std::endl;
            fileDialog.ClearSelected();

            scriptingEnabled = false;
            GravityScripting::Cleanup();
            GravityScripting::Initialize();
            GravityScripting::RegisterAPIFunctions();
            GravityScripting::CompileScriptFromFile(fileName);
            GravityScripting::RunScript("start", {}, 0);
            scriptingEnabled = true;
        }
    
        ImGui::End();
    }
#endif // IMGUI
}
