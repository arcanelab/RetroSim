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
    struct Renderer
    {
        GPU_Target *gpu;
    } renderer;

    struct Texture
    {
        GPU_Image *gpu;
    } texture;

    SDL_Window *window;

    uint32_t shader;
    GPU_ShaderBlock block;

    bool quit = false;

    static void destoryTexture(Texture texture)
    {
        GPU_FreeImage(texture.gpu);
    }

    static void destoryRenderer(Renderer renderer)
    {
        GPU_CloseCurrentRenderer();
    }

    static void renderClear(Renderer renderer)
    {
        GPU_Clear(renderer.gpu);
    }

    static void renderPresent(Renderer renderer)
    {
        GPU_Flip(renderer.gpu);
    }

    static void renderCopy(Renderer render, Texture tex, SDL_Rect src, SDL_Rect dst)
    {
        GPU_Rect gpusrc = {static_cast<float>(src.x), static_cast<float>(src.y),
                           static_cast<float>(src.w), static_cast<float>(src.h)};
        GPU_BlitScale(tex.gpu, &gpusrc, render.gpu, dst.x, dst.y, (float)dst.w / src.w, (float)dst.h / src.h);
    }

    void initGPU()
    {
        int32_t w, h;
        SDL_GetWindowSize(window, &w, &h);

        GPU_SetInitWindow(SDL_GetWindowID(window));

        GPU_SetPreInitFlags(GPU_INIT_ENABLE_VSYNC);
        renderer.gpu = GPU_Init(w, h, GPU_DEFAULT_INIT_FLAGS);

        GPU_SetWindowResolution(w, h);
        GPU_SetVirtualResolution(renderer.gpu, w, h);

        texture.gpu = GPU_CreateImage(GPU::windowWidth, GPU::windowHeight, GPU_FORMAT_RGBA);
        GPU_SetAnchor(texture.gpu, 0, 0);
        GPU_SetImageFilter(texture.gpu, GPU_FILTER_NEAREST);
    }

    static void loadCrtShader()
    {
        static const char VertextShader[] =
#if !defined(EMSCRIPTEN)
            "#version 110"
            "\n"
#endif
            "attribute vec3 gpu_Vertex;"
            "\n"
            "attribute vec2 gpu_TexCoord;"
            "\n"
            "attribute vec4 gpu_Color;"
            "\n"
            "uniform mat4 gpu_ModelViewProjectionMatrix;"
            "\n"
            "varying vec4 color;"
            "\n"
            "varying vec2 texCoord;"
            "\n"
            "void main(void)"
            "\n"
            "{"
            "\n"
            "    color = gpu_Color;"
            "\n"
            "    texCoord = vec2(gpu_TexCoord);"
            "\n"
            "    gl_Position = gpu_ModelViewProjectionMatrix * vec4(gpu_Vertex, 1.0);"
            "\n"
            "}"
            "\n";

        static const char PixelShader[] =
#if !defined(EMSCRIPTEN)
            "#version 110"
            "\n"
#else
            "precision highp float;"
            "\n"
#endif
            "varying vec2 texCoord;"
            "\n"
            "uniform sampler2D source;"
            "\n"
            "uniform float trg_x;"
            "\n"
            "uniform float trg_y;"
            "\n"
            "uniform float trg_w;"
            "\n"
            "uniform float trg_h;"
            "\n"
            ""
            "\n"
            "// Emulated input resolution."
            "\n"
            "vec2 res=vec2(1024.0,288.0);"
            "\n"
            ""
            "\n"
            "// Hardness of scanline."
            "\n"
            "//  -8.0 = soft"
            "\n"
            "// -16.0 = medium"
            "\n"
            "float hardScan=-8.0;"
            "\n"
            ""
            "\n"
            "// Hardness of pixels in scanline."
            "\n"
            "// -2.0 = soft"
            "\n"
            "// -4.0 = hard"
            "\n"
            "float hardPix=-2.0;"
            "\n"
            ""
            "\n"
            "// Display warp."
            "\n"
            "// 0.0 = none"
            "\n"
            "// 1.0/8.0 = extreme"
            "\n"
            "vec2 warp=vec2(1.0/64.0,1.0/48.0); "
            "\n"
            ""
            "\n"
            "// Amount of shadow mask."
            "\n"
            "float maskDark=0.9;"
            "\n"
            "float maskLight=1.1;"
            "\n"
            ""
            "\n"
            "//------------------------------------------------------------------------"
            "\n"
            ""
            "\n"
            "// sRGB to Linear."
            "\n"
            "// Assuing using sRGB typed textures this should not be needed."
            "\n"
            "float ToLinear1(float c){return(c<=0.04045)?c/12.92:pow((c+0.055)/1.055,2.4);}"
            "\n"
            "vec3 ToLinear(vec3 c){return vec3(ToLinear1(c.r),ToLinear1(c.g),ToLinear1(c.b));}"
            "\n"
            ""
            "\n"
            "// Linear to sRGB."
            "\n"
            "// Assuing using sRGB typed textures this should not be needed."
            "\n"
            "float ToSrgb1(float c){return(c<0.0031308?c*12.92:1.055*pow(c,0.41666)-0.055);}"
            "\n"
            "vec3 ToSrgb(vec3 c){return vec3(ToSrgb1(c.r),ToSrgb1(c.g),ToSrgb1(c.b));}"
            "\n"
            ""
            "\n"
            "// Nearest emulated sample given floating point position and texel offset."
            "\n"
            "// Also zero's off "
            "\n"
            "vec3 Fetch(vec2 pos,vec2 off){"
            "\n"
            "    pos=(floor(pos*res+off)+vec2(0.5,0.5))/res;"
            "\n"
            "    return ToLinear(1.2 * texture2D(source,pos.xy,-16.0).rgb);}"
            "\n"
            ""
            "\n"
            "// Distance in emulated pixels to nearest texel."
            "\n"
            "vec2 Dist(vec2 pos){pos=pos*res;return -((pos-floor(pos))-vec2(0.5));}"
            "\n"
            "        "
            "\n"
            "// 1D Gaussian."
            "\n"
            "float Gaus(float pos,float scale){return exp2(scale*pos*pos);}"
            "\n"
            ""
            "\n"
            "// 3-tap Gaussian filter along horz line."
            "\n"
            "vec3 Horz3(vec2 pos,float off){"
            "\n"
            "    vec3 b=Fetch(pos,vec2(-1.0,off));"
            "\n"
            "    vec3 c=Fetch(pos,vec2( 0.0,off));"
            "\n"
            "    vec3 d=Fetch(pos,vec2( 1.0,off));"
            "\n"
            "    float dst=Dist(pos).x;"
            "\n"
            "    // Convert distance to weight."
            "\n"
            "    float scale=hardPix;"
            "\n"
            "    float wb=Gaus(dst-1.0,scale);"
            "\n"
            "    float wc=Gaus(dst+0.0,scale);"
            "\n"
            "    float wd=Gaus(dst+1.0,scale);"
            "\n"
            "    // Return filtered sample."
            "\n"
            "    return (b*wb+c*wc+d*wd)/(wb+wc+wd);}"
            "\n"
            ""
            "\n"
            "// 5-tap Gaussian filter along horz line."
            "\n"
            "vec3 Horz5(vec2 pos,float off){"
            "\n"
            "    vec3 a=Fetch(pos,vec2(-2.0,off));"
            "\n"
            "    vec3 b=Fetch(pos,vec2(-1.0,off));"
            "\n"
            "    vec3 c=Fetch(pos,vec2( 0.0,off));"
            "\n"
            "    vec3 d=Fetch(pos,vec2( 1.0,off));"
            "\n"
            "    vec3 e=Fetch(pos,vec2( 2.0,off));"
            "\n"
            "    float dst=Dist(pos).x;"
            "\n"
            "    // Convert distance to weight."
            "\n"
            "    float scale=hardPix;"
            "\n"
            "    float wa=Gaus(dst-2.0,scale);"
            "\n"
            "    float wb=Gaus(dst-1.0,scale);"
            "\n"
            "    float wc=Gaus(dst+0.0,scale);"
            "\n"
            "    float wd=Gaus(dst+1.0,scale);"
            "\n"
            "    float we=Gaus(dst+2.0,scale);"
            "\n"
            "    // Return filtered sample."
            "\n"
            "    return (a*wa+b*wb+c*wc+d*wd+e*we)/(wa+wb+wc+wd+we);}"
            "\n"
            ""
            "\n"
            "// Return scanline weight."
            "\n"
            "float Scan(vec2 pos,float off){"
            "\n"
            "    float dst=Dist(pos).y;"
            "\n"
            "    return Gaus(dst+off,hardScan);}"
            "\n"
            ""
            "\n"
            "// Allow nearest three lines to effect pixel."
            "\n"
            "vec3 Tri(vec2 pos){"
            "\n"
            "    vec3 a=Horz3(pos,-1.0);"
            "\n"
            "    vec3 b=Horz5(pos, 0.0);"
            "\n"
            "    vec3 c=Horz3(pos, 1.0);"
            "\n"
            "    float wa=Scan(pos,-1.0);"
            "\n"
            "    float wb=Scan(pos, 0.0);"
            "\n"
            "    float wc=Scan(pos, 1.0);"
            "\n"
            "    return a*wa+b*wb+c*wc;}"
            "\n"
            ""
            "\n"
            "// Distortion of scanlines, and end of screen alpha."
            "\n"
            "vec2 Warp(vec2 pos){"
            "\n"
            "    pos=pos*2.0-1.0;    "
            "\n"
            "    pos*=vec2(1.0+(pos.y*pos.y)*warp.x,1.0+(pos.x*pos.x)*warp.y);"
            "\n"
            "    return pos*0.5+0.5;}"
            "\n"
            ""
            "\n"
            "// Shadow mask."
            "\n"
            "vec3 Mask(vec2 pos){"
            "\n"
            "    pos.x+=pos.y*3.0;"
            "\n"
            "    vec3 mask=vec3(maskDark,maskDark,maskDark);"
            "\n"
            "    pos.x=fract(pos.x/6.0);"
            "\n"
            "    if(pos.x<0.333)mask.r=maskLight;"
            "\n"
            "    else if(pos.x<0.666)mask.g=maskLight;"
            "\n"
            "    else mask.b=maskLight;"
            "\n"
            "    return mask;}    "
            "\n"
            ""
            "\n"
            "void main() {"
            "\n"
            "    hardScan=-12.0;"
            "\n"
            "    //maskDark=maskLight;"
            "\n"
            "    vec2 start=gl_FragCoord.xy-vec2(trg_x, trg_y);"
            "\n"
            "    start.y=trg_h-start.y;"
            "\n"
            ""
            "\n"
            "    vec2 pos=Warp(start/vec2(trg_w, trg_h));"
            "\n"
            "    gl_FragColor.rgb=Tri(pos)*Mask(gl_FragCoord.xy);"
            "\n"
            ""
            "\n"
            "    gl_FragColor = vec4(ToSrgb(gl_FragColor.rgb), 1.0);"
            "\n"
            "}"
            "\n";

        uint32_t vertex = GPU_CompileShader(GPU_VERTEX_SHADER, VertextShader);

        if (!vertex)
        {
            printf("Failed to load vertex shader: %s\n", GPU_GetShaderMessage());
            return;
        }

        uint32_t pixel = GPU_CompileShader(GPU_PIXEL_SHADER, PixelShader);

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
            block = GPU_LoadShaderBlock(shader, "gpu_Vertex", "gpu_TexCoord", "gpu_Color", "gpu_ModelViewProjectionMatrix");
            GPU_ActivateShaderProgram(shader, &block);
        }
        else
        {
            printf("Failed to link shader program: %s\n", GPU_GetShaderMessage());
        }
    }

    static void pollEvents()
    {

        SDL_Event event;

        // Workaround for freeze on fullscreen under macOS #819
        SDL_PumpEvents();
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_WINDOWEVENT:
                switch (event.window.event)
                {
                case SDL_WINDOWEVENT_SIZE_CHANGED:
                {
                    int32_t w, h;
                    SDL_GetWindowSize(window, &w, &h);
                    GPU_SetWindowResolution(w, h);
                    GPU_SetVirtualResolution(renderer.gpu, w, h);
                }
                break;
                }
                break;

            case SDL_KEYDOWN:
                if (event.key.keysym.sym == SDLK_ESCAPE)
                {
                    quit = true;
                }
                break;
            case SDL_QUIT:
                break;
            default:
                break;
            }
        }
    }

    static void calcTextureRect(SDL_Rect *rect)
    {
        bool integerScale = false;

        int32_t sw, sh, w, h;
        SDL_GetWindowSize(window, &sw, &sh);

        enum
        {
            Width = GPU::windowWidth,
            Height = GPU::windowHeight
        };

        if (sw * Height < sh * Width)
        {
            w = sw - (integerScale ? sw % Width : 0);
            h = Height * w / Width;
        }
        else
        {
            h = sh - (integerScale ? sh % Height : 0);
            w = Width * h / Height;
        }

        *rect = (SDL_Rect)
        {
            (sw - w) / 2,
#if defined(TOUCH_INPUT_SUPPORT)
                // snap the screen up to get a place for the software keyboard
                sw > sh ? (sh - h) / 2 : 0,
#else
                (sh - h) / 2,
#endif
                w, h
        };
    }

    int GetScreenRefreshRate()
    {
        return 120;
    }
    
    static void gpuTick()
    {
        pollEvents();

        renderClear(renderer);

        // updateTextureBytes(texture, tic->product.screen, TIC80_FULLWIDTH, TIC80_FULLHEIGHT);

        SDL_Rect rect;
        calcTextureRect(&rect);

        if (shader == 0)
            loadCrtShader();

        GPU_ActivateShaderProgram(shader, &block);

        static const char *Uniforms[] = {"trg_x", "trg_y", "trg_w", "trg_h"};

        for (int32_t i = 0; i < COUNT_OF(Uniforms); ++i)
            GPU_SetUniformf(GPU_GetUniformLocation(shader, Uniforms[i]), (&rect.x)[i]);

        GPU_BlitScale(texture.gpu, NULL, renderer.gpu, rect.x, rect.y,
                      (float)rect.w / GPU::windowWidth, (float)rect.h / GPU::windowHeight);
        GPU_DeactivateShaderProgram();

        renderPresent(renderer);
    }

    void Run(std::string stringFilename)
    {
        int result = SDL_Init(SDL_INIT_VIDEO);
        if (result != 0)
        {
            SDL_Log("Unable to initialize SDL Video: %i, %s\n", result, SDL_GetError());
            return;
        }

        int32_t flags = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL;
        window = SDL_CreateWindow("RetroSim", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, GPU::windowWidth, GPU::windowHeight, flags);

        {
            const uint64_t Delta = SDL_GetPerformanceFrequency() / GetScreenRefreshRate();
            uint64_t nextTick = SDL_GetPerformanceCounter();

            while (!quit)
            {
                gpuTick();

                int64_t delay = (nextTick += Delta) - SDL_GetPerformanceCounter();

                if (delay > 0)
                    SDL_Delay((uint32_t)(delay * 1000 / SDL_GetPerformanceFrequency()));
            }
        }
    }

}