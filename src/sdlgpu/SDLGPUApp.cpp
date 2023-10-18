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
    GPU_Target *gpuRenderTarget; // renderer
    GPU_Image *gpuScreenTexture; // screen texture
    SDL_Window *window;

    uint32_t shader;
    GPU_ShaderBlock shaderBlock;

    void RenderClear(GPU_Target *target)
    {
        GPU_Clear(target);
    }

    void RenderPresent(GPU_Target *target)
    {
        GPU_Flip(target);
    }

    void RenderCopy(GPU_Target *target, GPU_Image *texture, GPU_Rect *srcRect, GPU_Rect *dstRect)
    {
        GPU_BlitScale(texture, srcRect, target, dstRect->x, dstRect->y, dstRect->w, dstRect->h);
        // GPU_Blit(texture, srcRect, target, dstRect->x, dstRect->y);
    }

    void UpdateTextureBytes(GPU_Image *texture, const void *data, int width, int height)
    {
        GPU_UpdateImageBytes(texture, nullptr, (uint8_t *)data, width * sizeof(uint32_t));
    }

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

    void LoadShader()
    {
        std::string shaderPath = Core::GetInstance()->GetCoreConfig().GetDataPath() + "/shaders/";
        RetroSim::Logger::LogPrintf(RETRO_LOG_INFO, "Loading shaders from %s\n", shaderPath.c_str());

        // print directory contents in C++
        namespace fs = std::filesystem;

        for (const auto &entry : fs::directory_iterator(shaderPath))
        {
            if (entry.is_regular_file())
            {
                std::cout << "Found file: " << entry.path() << std::endl;
            }
            else if (entry.is_directory())
            {
                std::cout << "Found directory: " << entry.path() << std::endl;
            }
        }

        string PixelShader = RetroSim::ReadTextFile(shaderPath + "fragment.glsl");
        string VertexShader = RetroSim::ReadTextFile(shaderPath + "vertex.glsl");

        // printf("Vertex shader:\n%s\n", RetroSim::ReadTextFile(shaderPath + "vertex.glsl").c_str());

        uint32_t vertex = GPU_CompileShader(GPU_VERTEX_SHADER, VertexShader.c_str());

        if (!vertex)
        {
            printf("Failed to load vertex shader: %s\n", GPU_GetShaderMessage());
            return;
        }

        uint32_t pixel = GPU_CompileShader(GPU_PIXEL_SHADER, PixelShader.c_str());

        if (!pixel)
        {
            printf("Failed to load pixel shader: %s\n", GPU_GetShaderMessage());
            return;
        }

        if (shader)
            GPU_FreeShaderProgram(shader);

        shader = GPU_LinkShaders(vertex, pixel);

        if (shader)
        {
            shaderBlock = GPU_LoadShaderBlock(shader, "gpu_Vertex", "gpu_TexCoord", "gpu_Color", "gpu_ModelViewProjectionMatrix");
            GPU_ActivateShaderProgram(shader, &shaderBlock);
        }
        else
        {
            printf("Failed to link shader program: %s\n", GPU_GetShaderMessage());
        }
    }

    void printRenderers(void)
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

    void Run(std::string scriptFileName)
    {
        int error = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
        if (error != 0)
        {
            printf("Failed to initialize SDL: %s\n", SDL_GetError());
            return;
        }

        char basePath[] = ".";
        Core *core = Core::GetInstance();

        core->Initialize(basePath);

        int scalingFactor = core->GetCoreConfig().GetWindowScale();

        int originalWidth = GPU::windowWidth;
        int originalHeight = GPU::windowHeight;

        int scaledWidth = originalWidth * scalingFactor;
        int scaledHeight = originalHeight * scalingFactor;

        window = SDL_CreateWindow("RetroSim", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, scaledWidth, scaledHeight, SDL_WINDOW_OPENGL);
        // int width, height;
        // SDL_GetWindowSize(window, &width, &height);
        printRenderers();

        uint32_t windowId = SDL_GetWindowID(window);

        GPU_SetInitWindow(windowId);
        GPU_SetPreInitFlags(GPU_INIT_ENABLE_VSYNC);
        gpuRenderTarget = GPU_Init(scaledWidth, scaledHeight, GPU_DEFAULT_INIT_FLAGS);
        if (gpuRenderTarget == NULL)
        {
            printf("Failed to create renderer\n");
            return;
        }
        // GPU_SetWindowResolution(GPU::windowWidth, GPU::windowHeight);
        // GPU_SetVirtualResolution(gpuRenderTarget, GPU::windowWidth, GPU::windowHeight);
        GPU_SetVirtualResolution(gpuRenderTarget, GPU::textureWidth, GPU::textureHeight);
        gpuScreenTexture = GPU_CreateImage(GPU::windowWidth, GPU::windowHeight, GPU_FORMAT_RGBA);
        GPU_SetAnchor(gpuScreenTexture, 0, 0);
        GPU_SetImageFilter(gpuScreenTexture, GPU_FILTER_NEAREST); // GPU_FILTER_LINEAR, GPU_FILTER_NEAREST

        LoadShader();

        int refreshRate = 120;
        if (refreshRate != -1)
            Core::GetInstance()->SetRefreshRate(refreshRate);

        if (scriptFileName.empty() == false)
            Core::GetInstance()->GetCoreConfig().SetScriptPath(scriptFileName);

        SDL_Rect rect;
        rect.x = 0;
        rect.y = 0;
        rect.w = scaledWidth;
        rect.h = scaledHeight;

        int borderWidth = (GPU::windowWidth - GPU::textureWidth) / 2;
        int borderHeight = (GPU::windowHeight - GPU::textureHeight) / 2;

        GPU_Rect gpuRect;
        gpuRect.x = borderWidth;
        gpuRect.y = borderHeight;
        gpuRect.w = GPU::textureWidth;
        gpuRect.h = GPU::textureHeight;

        SDL_Rect shaderRect;
        shaderRect.x = borderWidth;
        shaderRect.y = borderHeight;
        shaderRect.w = GPU::textureWidth;
        shaderRect.h = GPU::textureHeight;

        bool quit = false;
        SDL_Event event;
        while (!quit)
        {
            quit = PollEvents(event);
            // GPU_Clear(gpuRenderTarget);
            Core::GetInstance()->RunNextFrame();
            // copy texture to screen
            GPU_UpdateImageBytes(gpuScreenTexture, &gpuRect, (uint8_t *)GPU::outputTexture, GPU::textureWidth * sizeof(uint32_t));
            GPU_ActivateShaderProgram(shader, &shaderBlock);

            static const char *Uniforms[] = {"trg_x", "trg_y", "trg_w", "trg_h"};

            for (int i = 0; i < COUNT_OF(Uniforms); ++i)
                GPU_SetUniformf(GPU_GetUniformLocation(shader, Uniforms[i]), (&rect.x)[i]);

            GPU_Blit(gpuScreenTexture, NULL, gpuRenderTarget, 0, 0);

            GPU_DeactivateShaderProgram();
            GPU_Flip(gpuRenderTarget);
        }
    }
}