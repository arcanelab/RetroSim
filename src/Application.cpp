// RetroSim - Copyright 2011-2023 Zoltán Majoros. All rights reserved.
// https://github.com/arcanelab

#include "Application.h"
#include <stdio.h>
#include <cstring>

void Application::Run()
{
    CreateSDLWindow();
    RunMainLoop();
}

void Application::RunMainLoop()
{
    printf("RunMainLoop\n");

    SDL_Event event;
    bool quit = false;
    /*
    core.Render();
    SDL_UpdateTexture(texture, NULL, core.gpu->outputTexture, core.gpu->width * sizeof(uint32_t));

    // Copy the texture to the renderer and present it to the screen
    if (SDL_RenderClear(renderer) != 0)
    {
        // Handle error
        printf("Failed to clear renderer: %s\n", SDL_GetError());
        return;
    }
    if (SDL_RenderCopy(renderer, texture, NULL, NULL) != 0)
    {
        // Handle error
        printf("Failed to copy texture to renderer: %s\n", SDL_GetError());
        return;
    }
    SDL_RenderPresent(renderer);
    */
    while (!quit)
    {
        core.Render();
        SDL_UpdateTexture(texture, NULL, core.gpu->outputTexture, core.gpu->width * sizeof(uint32_t));
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
    SDL_Init(SDL_INIT_EVERYTHING);
    window = SDL_CreateWindow("RetroSim", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                              core.gpu->width, core.gpu->height, SDL_WINDOW_SHOWN);

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,
                                core.gpu->width, core.gpu->height);

    // check for errors
    if (window == NULL)
    {
        printf("Could not create window: %s\n", SDL_GetError());
        exit(1);
    }
    if (renderer == NULL)
    {
        printf("Could not create renderer: %s\n", SDL_GetError());
        exit(1);
    }
    if (texture == NULL)
    {
        printf("Could not create texture: %s\n", SDL_GetError());
        exit(1);
    }
}
