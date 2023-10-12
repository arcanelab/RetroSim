// RetroSim - Copyright 2011-2023 Zoltán Majoros. All rights reserved.
// https://github.com/arcanelab

#pragma once

#include <string>
#include <mutex>
#include "CoreConfig.h"
#include "A65000CPU.h"
#include "CPUShader.h"

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
        void SetWindowSize(uint16_t width, uint16_t height);

        std::mutex memoryMutex;

        uint32_t *shadedTexture = nullptr;
        uint8_t scaleValue = 3;
        
    private:
        static Core *instance;
        CoreConfig coreConfig;
        bool isInitialized = false;
        uint32_t frameCounter;
        A65000CPU cpu;
        bool scriptingEnabled = false;

        uint16_t windowWidth;
        uint16_t windowHeight;
        CPUShader::ShaderParams shaderParams;

        void InitializeFonts();
        void InitializePalette();
        void InitializeCPU();
        void UpdateRegisters();
        static void SyscallHandler(uint16_t syscallID, uint32_t argumentAddress);
    };
}