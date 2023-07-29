// RetroSim - Copyright 2011-2023 Zoltán Majoros. All rights reserved.
// https://github.com/arcanelab

#pragma once
#include <string>

namespace RetroSim::Config
{
    struct ConfigValues
    {
        std::string scriptPath;
        bool fullscreen;
        int fpsOverride;
    };

    extern ConfigValues config;

    void Initialize();
};