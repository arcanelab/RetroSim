// RetroSim - Copyright 2011-2023 Zoltán Majoros. All rights reserved.
// https://github.com/arcanelab

#pragma once
#include "Core.h"
#include "GPU.h"
#include <SDL.h>

class Application
{
public:
    Application()
    {
        gpu = new GPU(core);
    };
    ~Application(){};

    void Run();

private:
    Core core;
    GPU *gpu;

    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;

    void CreateSDLWindow();
    void RunMainLoop();
};
