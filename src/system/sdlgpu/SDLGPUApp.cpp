// RetroSim - Copyright 2011-2023 Zoltán Majoros. All rights reserved.
// https://github.com/arcanelab

#pragma once
#include <string>
#include <SDL.h>
#include <SDL_gpu.h>
#include <filesystem>
#include <iostream>
#include "CoreConfig.h"
#include "Core.h"
#include "GPU.h"
#include "FileUtils.h"
#include "MMU.h"

#define COUNT_OF(x) ((sizeof(x) / sizeof(0 [x])) / ((size_t)(!(sizeof(x) % sizeof(0 [x])))))

namespace RetroSim::SDLGPUApp
{
    GPU_Target *windowRenderTarget; // renderer
    GPU_Image *screenTexture;    // screen texture
    GPU_Image *upscaledTexture;
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

    void SetShaderParameters()
    {
        MMU::memory.shaderParameters.MASK = 3.0f;
        MMU::memory.shaderParameters.MASK_INTENSITY = 0.3f;
        MMU::memory.shaderParameters.SCANLINE_STRENGTH = 0.5f;
        MMU::memory.shaderParameters.SCAN_BLUR = 6.0f;
        MMU::memory.shaderParameters.CURVATURE = 0.02f;
        MMU::memory.shaderParameters.TRINITRON_CURVE = 0.0f;
        MMU::memory.shaderParameters.CORNER = 9.0f;
        MMU::memory.shaderParameters.CRT_GAMMA = 2.4f;        
    }

    void LoadShaders()
    {
        std::string shaderPath = Core::GetInstance()->GetCoreConfig().GetDataPath() + "/shaders/";
        RetroSim::Logger::LogPrintf(RETRO_LOG_INFO, "Loading shaders from %s\n", shaderPath.c_str());

        string pixelShaderSource = RetroSim::ReadTextFile(shaderPath + "crt-lottes-fast-fragment.glsl");
        string vertexShaderSource = RetroSim::ReadTextFile(shaderPath + "crt-lottes-fast-vertex.glsl");

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
            // shaderBlock = GPU_LoadShaderBlock(linkedShaders, "gpu_Vertex", "gpu_TexCoord", "gpu_Color", "gpu_ModelViewProjectionMatrix");
            shaderBlock = GPU_LoadShaderBlock(linkedShaders, "VertexCoord", "TexCoord", "COLOR", "MVPMatrix");
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
        if (SDL_GetDesktopDisplayMode(0, &displayMode) == 0)
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
        if (InitializeSDL())
            return;

        char basePath[] = ".";
        Core *core = Core::GetInstance();

        core->Initialize(basePath);

        int windowScalingFactor = core->GetCoreConfig().GetWindowScale();

        int scaledWindowWidth = GPU::windowWidth * windowScalingFactor;
        int scaledWindowHeight = GPU::windowHeight * windowScalingFactor;

        // float internalScale = windowScalingFactor; // 3.0f;
        // int internalContentWidth = GPU::textureWidth * internalScale;
        // int internalContentHeight = GPU::textureHeight * internalScale;

        // int internalWindowWidth = GPU::windowWidth * internalScale;
        // int internalWindowHeight = GPU::windowHeight * internalScale;

        window = SDL_CreateWindow("RetroSim", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                  scaledWindowWidth, scaledWindowHeight,
                                  SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
        int width, height;
        SDL_GetWindowSize(window, &width, &height);
        LogPrintf(RETRO_LOG_INFO, "Window size: %dx%d\n", width, height);
        LogPrintf(RETRO_LOG_INFO, "GPU::windowSize: %dx%d\n", GPU::windowWidth, GPU::windowHeight);
        // print GPU::windowWidth and GPU::windowHeight

        // SDL_SetWindowSize(window, width, height);
        // printRenderers();

        SDL_GL_GetDrawableSize(window, &width, &height);
        float desktopScale = (float)width / (float)GPU::windowWidth / (float)windowScalingFactor;
        LogPrintf(RETRO_LOG_INFO, "OS scale: %f\n", desktopScale);
        LogPrintf(RETRO_LOG_INFO, "Renderer size: %dx%d\n", width, height);

        uint32_t windowId = SDL_GetWindowID(window);

        GPU_SetInitWindow(windowId);
        GPU_SetPreInitFlags(GPU_INIT_ENABLE_VSYNC);
        windowRenderTarget = GPU_InitRenderer(GPU_RENDERER_OPENGL_2, scaledWindowWidth, scaledWindowHeight, GPU_DEFAULT_INIT_FLAGS);
        if (windowRenderTarget == NULL)
        {
            printf("Failed to create renderer\n");
            return;
        }

        GPU_SetWindowResolution(scaledWindowWidth, scaledWindowHeight);
        // GPU_SetVirtualResolution(windowRenderTarget, GPU::windowWidth, GPU::windowHeight);
        // GPU_UnsetVirtualResolution(windowRenderTarget);
        screenTexture = GPU_CreateImage(GPU::windowWidth, GPU::windowHeight, GPU_FORMAT_RGBA);
        GPU_SetAnchor(screenTexture, 0, 0);
        GPU_SetImageFilter(screenTexture, GPU_FILTER_NEAREST); // GPU_FILTER_LINEAR, GPU_FILTER_NEAREST

        // upscaledTexture = GPU_CreateImage(GPU::windowWidth * desktopScale, GPU::windowHeight * desktopScale, GPU_FORMAT_RGBA);
        upscaledTexture = GPU_CreateImage(scaledWindowWidth * desktopScale, scaledWindowHeight * desktopScale, GPU_FORMAT_RGBA);
        GPU_SetImageFilter(upscaledTexture, GPU_FILTER_NEAREST);
        GPU_SetAnchor(upscaledTexture, 0, 0);
        GPU_Target* upscaledTarget = GPU_LoadTarget(upscaledTexture);
        GPU_SetBlendMode(screenTexture, GPU_BLEND_NORMAL);

        // print screentexture size
        LogPrintf(RETRO_LOG_INFO, "Screen texture size: %dx%d\n", GPU::windowWidth, GPU::windowHeight);
        // print upscaled texture size
        LogPrintf(RETRO_LOG_INFO, "Upscaled texture size: %dx%d, %dx%d\n", upscaledTexture->base_w, upscaledTexture->base_h, upscaledTexture->texture_w, upscaledTexture->texture_h);

        LoadShaders();
        SetShaderParameters();

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

        GPU_Rect windowRect;
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
            GPU_UpdateImageBytes(screenTexture, &contentRect, (uint8_t *)GPU::outputTexture, GPU::textureWidth * sizeof(uint32_t));
            GPU_BlitScale(screenTexture, NULL, upscaledTarget, 0, 0, windowScalingFactor * desktopScale, windowScalingFactor * desktopScale);

            // Set up shader variables
            GPU_ActivateShaderProgram(linkedShaders, &shaderBlock);
            static const char *Uniforms[] = {"OutputSize", "TextureSize", "InputSize", "CRT_GAMMA", "SCANLINE_STRENGTH", "SCAN_BLUR", "MASK_INTENSITY", "CURVATURE", "CORNER", "MASK", "TRINITRON_CURVE"};
            float outputSize[] = {windowRect.w * desktopScale, windowRect.h * desktopScale};
            GPU_SetUniformfv(GPU_GetUniformLocation(linkedShaders, "OutputSize"), 2, 1, outputSize);
            float textureSize[] = {windowRect.w, windowRect.h};
            GPU_SetUniformfv(GPU_GetUniformLocation(linkedShaders, "TextureSize"), 2, 1, textureSize);
            float inputSize[] = {windowRect.w, windowRect.h};
            GPU_SetUniformfv(GPU_GetUniformLocation(linkedShaders, "InputSize"), 2, 1, inputSize);
            GPU_SetUniformf(GPU_GetUniformLocation(linkedShaders, "MASK"), MMU::memory.shaderParameters.MASK);
            GPU_SetUniformf(GPU_GetUniformLocation(linkedShaders, "CRT_GAMMA"), MMU::memory.shaderParameters.CRT_GAMMA);
            GPU_SetUniformf(GPU_GetUniformLocation(linkedShaders, "SCANLINE_STRENGTH"), MMU::memory.shaderParameters.SCANLINE_STRENGTH);
            GPU_SetUniformf(GPU_GetUniformLocation(linkedShaders, "SCAN_BLUR"), MMU::memory.shaderParameters.SCAN_BLUR);
            GPU_SetUniformf(GPU_GetUniformLocation(linkedShaders, "MASK_INTENSITY"), MMU::memory.shaderParameters.MASK_INTENSITY);
            GPU_SetUniformf(GPU_GetUniformLocation(linkedShaders, "CURVATURE"), MMU::memory.shaderParameters.CURVATURE);
            GPU_SetUniformf(GPU_GetUniformLocation(linkedShaders, "CORNER"), MMU::memory.shaderParameters.CORNER);

            // copy rendered screen to window render target
            GPU_Blit(upscaledTexture, NULL, windowRenderTarget, 0, 0);
            // GPU_BlitScale(upscaledTexture, NULL, windowRenderTarget, 0, 0, 1.0f / desktopScale, 1.0f / desktopScale);
 
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