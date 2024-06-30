// RetroSim - Copyright 2011-2023 Zoltán Majoros. All rights reserved.
// https://github.com/arcanelab

#include "raylib.h"
#include "Core.h"
#include "GPU.h"
#include "Logger.h"
#include "FileUtils.h"
#include "DesktopDPI.h"
#include "RaylibShader.h"
#ifdef IMGUI
#include "CoreImGui.h"
#endif

#if defined(PLATFORM_ANDROID) || defined(PLATFORM_WEB)
#define GLSL_VERSION 100
#else
#define GLSL_VERSION 330
#endif

namespace RetroSim
{
    class RaylibApp
    {
    public:
        void Run();

    private:
        Core *core;
        int scaledWindowWidth;
        int scaledWindowHeight;
        int windowScalingFactor;
        float desktopScalingFactor;
        float effectiveScalingFactor;

        Raylib::RaylibShader shader;

        void InitializeWindow();
        void DrawImgui();

#ifdef IMGUI
        CoreImGui *coreImGui;
#endif
    };
}
