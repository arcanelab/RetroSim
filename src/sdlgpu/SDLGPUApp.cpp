// RetroSim - Copyright 2011-2023 Zoltán Majoros. All rights reserved.
// https://github.com/arcanelab

#pragma once
#include <string>
#include <SDL_gpu.h>
#include "CoreConfig.h"
#include "Core.h"
#include "GPU.h"

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
        //GPU_Blit(texture, srcRect, target, dstRect->x, dstRect->y);
    }

    void UpdateTextureBytes(GPU_Image *texture, const void *data, int width, int height)
    {
        GPU_UpdateImageBytes(texture, nullptr, (uint8_t *)data, width* sizeof(uint32_t));
    }
}