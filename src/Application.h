// RetroSim - Copyright 2011-2023 Zoltán Majoros. All rights reserved.
// https://github.com/arcanelab

#pragma once
#include "Core.h"
#include "GPU.h"
#include "ScriptManager.h"
#include "Config.h"
#include <SDL.h>
#include <string>

class Application
{
public:
    Application() {}
    ~Application(){};

    void Run(std::string stringFilename);

private:
    Core core;
    Config config;
    ScriptManager scriptManager;
    bool scriptingEnabled = false;

    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;

    void CreateSDLWindow();
    void RunMainLoop();
};
