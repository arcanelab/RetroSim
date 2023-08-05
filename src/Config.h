// RetroSim - Copyright 2011-2023 Zoltán Majoros. All rights reserved.
// https://github.com/arcanelab

#pragma once
#include <string>

namespace RetroSim::Config
{
    struct ConfigValues
    {
        bool isInitialized = false;
        std::string scriptPath;
        std::string dataPath;
        bool fullscreen = false;
        int fpsOverride = 0;
        const int width = 480;
        const int height = 256;
        int fps = 60;
    };

    extern ConfigValues config;

    void Initialize();
}