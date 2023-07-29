// RetroSim - Copyright 2011-2023 Zoltán Majoros. All rights reserved.
// https://github.com/arcanelab

#pragma once
#include <string>

class Config
{
public:
    std::string scriptPath;
    bool fullscreen;
    int fpsOverride;

    Config()
    {
        // Set default values in case they can't be read from the config file.
        scriptPath = "";
        fullscreen = false;
        fpsOverride = 0;

        LoadConfigFile();
    }

    void LoadConfigFile();
};