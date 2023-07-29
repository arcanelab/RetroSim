// RetroSim - Copyright 2011-2023 Zoltán Majoros. All rights reserved.
// https://github.com/arcanelab

#include "Application.h"
#include <stdio.h>
#include <cstring>
#include <string>

void Application::Run(std::string scriptFileName)
{
    CreateSDLWindow();
    RunMainLoop();
}

void Application::RunMainLoop()
{
    if (config.scriptPath.empty() == false)
    {
        scriptManager.CompileScriptFromFile(config.scriptPath);
        scriptManager.RunScript("start", {}, 0);
        scriptingEnabled = true;
    }
    // else
    // scriptManager.CompileScript("func main() { System.print(\"Hello world from Gravity!\") return 42 }");

    SDL_Event event;
    bool quit = false;

    uint32_t lastFrameTime = 0;
    while (!quit)
    {
        if (scriptingEnabled)
            scriptManager.RunScript("update", {}, 0);

        uint32_t frameStartTime = SDL_GetTicks();
        core.Render();
        SDL_UpdateTexture(texture, NULL, core.gpu->outputTexture, core.gpu->textureWidth * sizeof(uint32_t));
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);

        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
                quit = true;

            if (event.type == SDL_KEYDOWN)
            {
                if (event.key.keysym.sym == SDLK_ESCAPE)
                {
                    quit = true;
                }
            }
        }
        lastFrameTime = SDL_GetTicks() - frameStartTime;
    }

    scriptManager.Destroy();

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void Application::CreateSDLWindow()
{
    SDL_Init(SDL_INIT_EVERYTHING);

    window = SDL_CreateWindow("RetroSim", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                              core.gpu->textureWidth, core.gpu->textureHeight,
                              SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,
                                core.gpu->textureWidth, core.gpu->textureHeight);

    // Set the logical size to maintain aspect ratio
    float aspectRatio = (float)core.gpu->textureWidth / (float)core.gpu->textureHeight;
    SDL_RenderSetLogicalSize(renderer, core.gpu->textureWidth, (int)(core.gpu->textureWidth / aspectRatio));

    // Set the scale to fit the window while maintaining aspect ratio
    int windowWidth, windowHeight;
    SDL_GetWindowSize(window, &windowWidth, &windowHeight);
    float scaleX = (float)windowWidth / (float)core.gpu->textureWidth;
    float scaleY = (float)windowHeight / (float)(core.gpu->textureWidth / aspectRatio);
    SDL_RenderSetScale(renderer, scaleX, scaleY);

    SDL_DisplayMode displayMode;
    SDL_GetDesktopDisplayMode(0, &displayMode);
    SDL_SetWindowSize(window, displayMode.w / 2, displayMode.h / 2);
    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

    if (config.fullscreen)
        SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);

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
