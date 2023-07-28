// RetroSim - Copyright 2011-2023 Zoltán Majoros. All rights reserved.
// https://github.com/arcanelab

#include "Application.h"
#include <stdio.h>
#include <cstring>

#include "gravity_compiler.h"
#include "gravity_macros.h"
#include "gravity_core.h"
#include "gravity_vm.h"

void errorCallback(gravity_vm *vm, error_type_t error_type, const char *message, error_desc_t error_desc, void *xdata)
{
    printf("Gravity error: %s\n", message);
}

void RunTestScript()
{
    const char *source_code = R"(
        func main()
        {
            System.print("Hello world from Gravity!")
            return 42
        }

        func update(dt)
        {
            System.print(dt)
        }

        )";

    gravity_delegate_t delegate = {.error_callback = errorCallback};
    gravity_compiler_t *compiler = gravity_compiler_create(&delegate);
    gravity_closure_t *closure = gravity_compiler_run(compiler, source_code, (uint32_t)strlen(source_code), 0, true, true);
    gravity_vm *vm = gravity_vm_new(&delegate);
    gravity_compiler_transfer(compiler, vm);
    gravity_compiler_free(compiler);

    if (gravity_vm_runmain(vm, closure))
    {
        gravity_value_t result = gravity_vm_result(vm);

        char buffer[512];
        gravity_value_dump(vm, result, buffer, sizeof(buffer));
        printf("%s\n", buffer);
    }

    gravity_vm_loadclosure(vm, closure);
    gravity_value_t updateFunction = gravity_vm_getvalue(vm, "update", strlen("update"));

    gravity_closure_t *updateClosure = VALUE_AS_CLOSURE(updateFunction);
    gravity_value_t deltaTime = VALUE_FROM_FLOAT(rand() / (float)RAND_MAX);
    gravity_value_t params[] = {deltaTime};

    gravity_vm_runclosure(vm, updateClosure, VALUE_FROM_NULL, params, 1);

    gravity_vm_free(vm);
    gravity_core_free();
}

void Application::Run()
{
    RunTestScript();
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
        SDL_UpdateTexture(texture, NULL, core.gpu->outputTexture, core.gpu->textureWidth * sizeof(uint32_t));
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
