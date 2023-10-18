// RetroSim - Copyright 2011-2023 Zoltán Majoros. All rights reserved.
// https://github.com/arcanelab

#pragma once
#include <string>
#include <SDL_gpu.h>
#include <filesystem>
#include <iostream>
#include "CoreConfig.h"
#include "Core.h"
#include "GPU.h"
#include "FileUtils.h"

#define COUNT_OF(x) ((sizeof(x) / sizeof(0 [x])) / ((size_t)(!(sizeof(x) % sizeof(0 [x])))))

namespace RetroSim::SDLGPUApp
{
    GPU_Target *windowRenderTarget; // renderer
    GPU_Image *gpuScreenTexture; // screen texture
    SDL_Window *window;

    uint32_t linkedShaders;
    GPU_ShaderBlock shaderBlock;

    bool PollEvents(SDL_Event &event)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
                return true;

            if (event.type == SDL_KEYDOWN)
            {
                if (event.key.keysym.sym == SDLK_ESCAPE)
                {
                    return true;
                }
            }
        }

        return false;
    }

    void LoadShaders()
    {
        std::string shaderPath = Core::GetInstance()->GetCoreConfig().GetDataPath() + "/shaders/";
        RetroSim::Logger::LogPrintf(RETRO_LOG_INFO, "Loading shaders from %s\n", shaderPath.c_str());

        string pixelShaderSource = RetroSim::ReadTextFile(shaderPath + "fragment.glsl");
        string vertexShaderSource = RetroSim::ReadTextFile(shaderPath + "vertex.glsl");

        uint32_t vertexShaderId = GPU_CompileShader(GPU_VERTEX_SHADER, vertexShaderSource.c_str());

        if (!vertexShaderId)
        {
            printf("Failed to load vertex shader: %s\n", GPU_GetShaderMessage());
            return;
        }

        uint32_t pixelShaderId = GPU_CompileShader(GPU_PIXEL_SHADER, pixelShaderSource.c_str());

        if (!pixelShaderId)
        {
            printf("Failed to load pixel shader: %s\n", GPU_GetShaderMessage());
            return;
        }

        if (linkedShaders)
            GPU_FreeShaderProgram(linkedShaders);

        linkedShaders = GPU_LinkShaders(vertexShaderId, pixelShaderId);

        if (linkedShaders)
        {
            shaderBlock = GPU_LoadShaderBlock(linkedShaders, "gpu_Vertex", "gpu_TexCoord", "gpu_Color", "gpu_ModelViewProjectionMatrix");
            GPU_ActivateShaderProgram(linkedShaders, &shaderBlock);
        }
        else
        {
            printf("Failed to link shader program: %s\n", GPU_GetShaderMessage());
        }
    }

    int GetScreenRefreshRate()
    {
        SDL_DisplayMode displayMode;
        if(SDL_GetDesktopDisplayMode(0, &displayMode) == 0)
        {
            printf("Display mode: %dx%d@%d\n", displayMode.w, displayMode.h, displayMode.refresh_rate);
            return displayMode.refresh_rate;
        }
        else
        {
            return -1;
        }
    }

    int InitializeSDL()
    {
        int error = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
        if (error != 0)
        {
            printf("Failed to initialize SDL: %s\n", SDL_GetError());
            return -1;
        }

        return 0;
    }

    void Run(std::string scriptFileName)
    {
        if(InitializeSDL()) return;

        char basePath[] = ".";
        Core *core = Core::GetInstance();

        core->Initialize(basePath);

        int windowScalingFactor = core->GetCoreConfig().GetWindowScale();

        int scaledWindowWidth = GPU::windowWidth * windowScalingFactor;
        int scaledWindowHeight = GPU::windowHeight * windowScalingFactor;

        float internalScale = windowScalingFactor;//3.0f;
        int internalContentWidth = GPU::textureWidth * internalScale;
        int internalContentHeight = GPU::textureHeight * internalScale;

        int internalWindowWidth = GPU::windowWidth * internalScale;
        int internalWindowHeight = GPU::windowHeight * internalScale;

        window = SDL_CreateWindow("RetroSim", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, scaledWindowWidth, scaledWindowHeight, SDL_WINDOW_OPENGL);
        // int width, height;
        // SDL_GetWindowSize(window, &width, &height);
        // printRenderers();

        uint32_t windowId = SDL_GetWindowID(window);

        GPU_SetInitWindow(windowId);
        GPU_SetPreInitFlags(GPU_INIT_ENABLE_VSYNC);
        windowRenderTarget = GPU_InitRenderer(GPU_RENDERER_OPENGL_2, internalWindowWidth, internalWindowHeight, GPU_DEFAULT_INIT_FLAGS);
        if (windowRenderTarget == NULL)
        {
            printf("Failed to create renderer\n");
            return;
        }

        // GPU_SetWindowResolution(GPU::windowWidth, GPU::windowHeight);
        // GPU_SetVirtualResolution(gpuRenderTarget, GPU::windowWidth, GPU::windowHeight);
        GPU_SetVirtualResolution(windowRenderTarget, GPU::textureWidth, GPU::textureHeight);
        gpuScreenTexture = GPU_CreateImage(GPU::windowWidth, GPU::windowHeight, GPU_FORMAT_RGBA);
        GPU_SetAnchor(gpuScreenTexture, 0, 0);
        GPU_SetImageFilter(gpuScreenTexture, GPU_FILTER_NEAREST); // GPU_FILTER_LINEAR, GPU_FILTER_NEAREST

        LoadShaders();

        int refreshRate = GetScreenRefreshRate();
        if (refreshRate != -1)
            Core::GetInstance()->SetRefreshRate(refreshRate);

        if (scriptFileName.empty() == false)
            Core::GetInstance()->GetCoreConfig().SetScriptPath(scriptFileName);

        int borderWidth = (GPU::windowWidth - GPU::textureWidth) / 2;
        int borderHeight = (GPU::windowHeight - GPU::textureHeight) / 2;

        GPU_Rect contentRect;
        contentRect.x = borderWidth;
        contentRect.y = borderHeight;
        contentRect.w = GPU::textureWidth;
        contentRect.h = GPU::textureHeight;

        SDL_Rect windowRect;
        windowRect.x = 0;
        windowRect.y = 0;
        windowRect.w = scaledWindowWidth;
        windowRect.h = scaledWindowHeight;

        bool quit = false;
        SDL_Event event;
        while (!quit)
        {
            quit = PollEvents(event);
            // clear screen
            GPU_Clear(windowRenderTarget);
            Core::GetInstance()->RunNextFrame();
            // copy texture to screen
            GPU_UpdateImageBytes(gpuScreenTexture, &contentRect, (uint8_t *)GPU::outputTexture, GPU::textureWidth * sizeof(uint32_t));

            // Set up shader variables
            GPU_ActivateShaderProgram(linkedShaders, &shaderBlock);
            static const char *Uniforms[] = {"trg_x", "trg_y", "trg_w", "trg_h", "scale"};
            GPU_SetUniformf(GPU_GetUniformLocation(linkedShaders, "trg_x"), windowRect.x);
            GPU_SetUniformf(GPU_GetUniformLocation(linkedShaders, "trg_y"), windowRect.y);
            GPU_SetUniformf(GPU_GetUniformLocation(linkedShaders, "trg_w"), windowRect.w);
            GPU_SetUniformf(GPU_GetUniformLocation(linkedShaders, "trg_h"), windowRect.h);
            GPU_SetUniformf(GPU_GetUniformLocation(linkedShaders, "scale"), (float)windowScalingFactor);

            // copy rendered screen to window render target
            GPU_Blit(gpuScreenTexture, NULL, windowRenderTarget, 0, 0);
            // GPU_BlitRotate(gpuScreenTexture, NULL, windowRenderTarget, 0, 0, 45.0f);
            // GPU_BlitScale(gpuContentTexture, NULL, windowRenderTarget, borderWidth, borderHeight, 1, 1);

            GPU_DeactivateShaderProgram();

            // Display rendertarget to window
            GPU_Flip(windowRenderTarget);
        }
    }

    void PrintRenderers(void)
    {
        SDL_version compiled;
        SDL_version linked;
        GPU_RendererID *renderers;
        int i;
        int order_size;
        GPU_RendererID order[GPU_RENDERER_ORDER_MAX];

        GPU_SetDebugLevel(GPU_DEBUG_LEVEL_MAX);

        compiled = GPU_GetCompiledVersion();
        linked = GPU_GetLinkedVersion();
        if (compiled.major != linked.major || compiled.minor != linked.minor || compiled.patch != linked.patch)
            GPU_Log("SDL_gpu v%d.%d.%d (compiled with v%d.%d.%d)\n", linked.major, linked.minor, linked.patch, compiled.major, compiled.minor, compiled.patch);
        else
            GPU_Log("SDL_gpu v%d.%d.%d\n", linked.major, linked.minor, linked.patch);

        renderers = (GPU_RendererID *)malloc(sizeof(GPU_RendererID) * GPU_GetNumRegisteredRenderers());
        GPU_GetRegisteredRendererList(renderers);

        GPU_Log("\nAvailable renderers:\n");
        for (i = 0; i < GPU_GetNumRegisteredRenderers(); i++)
        {
            GPU_Log("* %s (%d.%d)\n", renderers[i].name, renderers[i].major_version, renderers[i].minor_version);
        }
        GPU_Log("Renderer init order:\n");

        GPU_GetRendererOrder(&order_size, order);
        for (i = 0; i < order_size; i++)
        {
            GPU_Log("%d) %s (%d.%d)\n", i + 1, order[i].name, order[i].major_version, order[i].minor_version);
        }
        GPU_Log("\n");

        free(renderers);
    }
}