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
            "vec2 res=vec2(470.0 * 1.00,256.0 * 1.0);"
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
            // "vec2 warp=vec2(1.0/64.0,1.0/48.0); "
            "vec2 warp=vec2(0.0,0.0); "
            "\n"
            ""
            "\n"
            "// Amount of shadow mask."
            "\n"
            "float maskDark=0.95;"
            "\n"
            "float maskLight=1.05;"
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
            "// Also zero's off screen."
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
            // "    gl_FragColor = vec4(ToSrgb(gl_FragColor.rgb), 1.0);"
            "    gl_FragColor = vec4(texture2D(source,pos));"
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
        // GPU_Target *screen;

        // GPU_SetRequiredFeatures(GPU_FEATURE_BASIC_SHADERS);
        // screen = GPU_InitRenderer(GPU_RENDERER_OPENGL_1, 800, 600, GPU_DEFAULT_INIT_FLAGS);
        // if (screen == NULL)
        // {
        //     GPU_LogError("Initialization Error: Could not create a renderer with proper feature support for this demo.\n");
        //     return;
        // }

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
        GPU_SetImageFilter(gpuScreenTexture, GPU_FILTER_NEAREST);

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
            // GPU_UpdateImageBytes(gpuScreenTexture, nullptr, (uint8_t *)GPU::outputTexture, GPU::textureWidth * sizeof(uint32_t));
            GPU_UpdateImageBytes(gpuScreenTexture, &gpuRect, (uint8_t *)GPU::outputTexture, GPU::textureWidth * sizeof(uint32_t));
            GPU_ActivateShaderProgram(shader, &shaderBlock);

            static const char *Uniforms[] = {"trg_x", "trg_y", "trg_w", "trg_h"};

            for (int i = 0; i < COUNT_OF(Uniforms); ++i)
                GPU_SetUniformf(GPU_GetUniformLocation(shader, Uniforms[i]), (&rect.x)[i]);

            GPU_Blit(gpuScreenTexture, NULL, gpuRenderTarget, 0, 0);
            
            // GPU_BlitScale(gpuScreenTexture, NULL, gpuRenderTarget, gpuRect.x, gpuRect.y, 1, 1);
            // GPU_BlitScale(gpuScreenTexture, &gpuRect, gpuRenderTarget, 100, 100, 0.2f, 0.2f);

            GPU_DeactivateShaderProgram();
            GPU_Flip(gpuRenderTarget);
            SDL_Delay(8);
        }
    }
}