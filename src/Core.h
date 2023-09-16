// RetroSim - Copyright 2011-2023 Zoltán Majoros. All rights reserved.
// https://github.com/arcanelab

#pragma once

#include <string>
#include <mutex>
#include "CoreConfig.h"

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
        CoreConfig GetCoreConfig();
        void RunNextFrame();
        void Reset();
        void Shutdown();

        std::mutex memoryMutex;

    private:
        static Core *instance;
        CoreConfig coreConfig;

        void InitializeFonts();
        void InitializePalette();
    };
}