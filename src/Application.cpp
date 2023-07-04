// RetroSim - Copyright 2011-2023 Zoltán Majoros. All rights reserved.
// https://github.com/arcanelab

#include "Application.h"

void Application::Run()
{
    CreateSDLWindow();
    RunMainLoop();
}

void Application::RunMainLoop()
{
    SDL_Event event;
    bool quit = false;

    // core.mmu->WriteMem<uint8_t>(Core::TILE_MODE_U8, GPU::TILE_MODE_8x16);

    while (!quit)
    {
        core.Render();
        SDL_UpdateTexture(texture, NULL, gpu->outputTexture, gpu->width * sizeof(uint32_t));
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);

        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
                quit = true;
        }
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void Application::CreateSDLWindow()
{
    SDL_Init(SDL_INIT_VIDEO);
    window = SDL_CreateWindow("RetroSim",
                              SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED,
                              gpu->width,
                              gpu->height,
                              SDL_WINDOW_SHOWN);

    renderer = SDL_CreateRenderer(window, -1, 0);

    texture = SDL_CreateTexture(renderer,
                                SDL_PIXELFORMAT_RGBA8888,
                                SDL_TEXTUREACCESS_STREAMING,
                                gpu->width,
                                gpu->height);
}