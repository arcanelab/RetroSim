// RetroSim - Copyright 2011-2023 Zoltán Majoros. All rights reserved.
// https://github.com/arcanelab

#include "SDLApp.h"
#include "CoreConfig.h"
#include "Core.h"
#include "GPU.h"
#include <SDL.h>

using namespace RetroSim;

namespace RetroSim::SDLApp
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;

    void CreateSDLWindow();
    void RunMainLoop();

    void Run(std::string scriptFileName)
    {
        char basePath[] = ".";
        Core::GetInstance()->Initialize(basePath);

        int refreshRate = GetScreenRefreshRate();
        if(refreshRate != -1)
            Core::GetInstance()->SetRefreshRate(refreshRate);

        if (scriptFileName.empty() == false)
            Core::GetInstance()->GetCoreConfig().SetScriptPath(scriptFileName);

        CreateSDLWindow();
        RunMainLoop();
    }

    void RunMainLoop()
    {
        SDL_Event event;
        bool quit = false;

        uint32_t lastFrameTime = 0;
        SDL_Rect destinationRect = {(GPU::windowWidth - GPU::textureWidth) / 2, (GPU::windowHeight - GPU::textureHeight) / 2, GPU::textureWidth, GPU::textureHeight};
        while (!quit)
        {
            uint32_t frameStartTime = SDL_GetTicks();
            Core::GetInstance()->RunNextFrame();
            SDL_UpdateTexture(texture, NULL, GPU::outputTexture, GPU::textureWidth * sizeof(uint32_t));
            SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer, texture, NULL, &destinationRect);
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

        Core::GetInstance()->Shutdown();

        SDL_DestroyTexture(texture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }

    void CreateSDLWindow()
    {
        SDL_Init(SDL_INIT_EVERYTHING);

        window = SDL_CreateWindow("RetroSim", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                  GPU::windowWidth, GPU::windowHeight,
                                  SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);

        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

        texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING,
                                    GPU::textureWidth, GPU::textureHeight);

        // Set the logical size to maintain aspect ratio
        float aspectRatio = (float)GPU::windowWidth / (float)GPU::windowHeight;
        SDL_RenderSetLogicalSize(renderer, GPU::windowWidth, (int)(GPU::windowWidth / aspectRatio));

        // Set the scale to fit the window while maintaining aspect ratio
        int windowWidth, windowHeight;
        SDL_GetWindowSize(window, &windowWidth, &windowHeight);
        float scaleX = (float)windowWidth / (float)GPU::windowWidth;
        float scaleY = (float)windowHeight / (float)(GPU::windowWidth / aspectRatio);
        SDL_RenderSetScale(renderer, scaleX, scaleY);

        SDL_DisplayMode displayMode;
        SDL_GetDesktopDisplayMode(0, &displayMode);

        int scale = Core::GetInstance()->GetCoreConfig().GetWindowScale();
        SDL_SetWindowSize(window, GPU::windowWidth * scale, GPU::windowHeight * scale);
        SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

        if (Core::GetInstance()->GetCoreConfig().IsFullScreen())
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

    int GetScreenRefreshRate()
    {
        SDL_DisplayMode displayMode;
        if(SDL_GetDesktopDisplayMode(0, &displayMode) == 0)
            return displayMode.refresh_rate;
        else
            return -1;
    }
}