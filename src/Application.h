// RetroSim - Copyright 2011-2023 Zoltán Majoros. All rights reserved.
// https://github.com/arcanelab

#pragma once
#include "Core.h"
#include "GPU.h"
#include <SDL.h>

class Application
{
public:
    Application() {}
    ~Application(){};

    void Run();

private:
    Core core;

    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;

    void CreateSDLWindow();
    void RunMainLoop();
};
