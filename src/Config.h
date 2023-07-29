// RetroSim - Copyright 2011-2023 Zoltán Majoros. All rights reserved.
// https://github.com/arcanelab

#pragma once
#include <string>

class Config
{
public:
    std::string scriptPath;
    bool fullscreen;
    int forcedFPS;

    Config()
    {
        // Set default values in case they can't be read from the config file.
        scriptPath = "";
        fullscreen = false;
        forcedFPS = 0;

        LoadConfigFile();
    }

    void LoadConfigFile();
};