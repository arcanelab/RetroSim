// RetroSim - Copyright 2011-2023 Zoltán Majoros. All rights reserved.
// https://github.com/arcanelab

#pragma once

#include <string>
#include <mutex>
#include "CoreConfig.h"
#include "A65000CPU.h"

#ifdef IMGUI
class CoreImGui;
#endif

namespace RetroSim
{
    class Core
    {
    public:
        static Core *GetInstance()
        {
            if (instance == nullptr)
                instance = new Core();

            return instance;
        }

        void Initialize(const std::string &basePath);
        bool LoadCartridge(const std::string &path);
        void LoadRetroSimBinaryFile(const std::string &path);
        CoreConfig GetCoreConfig();
        void RunNextFrame();
        void Reset();
        void Shutdown();
        void SetRefreshRate(int refreshRate);
        void Pause();
        void Resume();
        bool IsPaused();

        void RenderAudio(uint16_t **audioBuffer, uint32_t *audioBufferSize);
        uint32_t GetSampleRate();

        std::mutex memoryMutex;

#ifdef IMGUI
        friend CoreImGui;
#endif // IMGUI

    private:
        static Core *instance;
        CoreConfig coreConfig;
        bool isInitialized = false;
        uint32_t frameCounter;
        A65000CPU cpu;
        bool scriptingEnabled = false;
        bool isPaused = false;

        void InitializeFonts();
        void InitializePalette();
        void InitializeCPU();
        void UpdateRegisters();
        static void SyscallHandler(uint16_t syscallID, uint32_t argumentAddress);
    };
}