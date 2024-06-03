// RetroSim - Copyright 2011-2023 Zoltán Majoros. All rights reserved.
// https://github.com/arcanelab

#include "raylib.h"
#include "Core.h"
#include "GPU.h"
#include "Logger.h"
#include "FileUtils.h"
#include "DesktopDPI.h"

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

        float hardScan = -8.0f;
        float hardPix = -3.0f;
        float warpX = 0.021f;
        float warpY = 0.031f;
        float maskDark = 0.5f;
        float maskLight = 1.5f;
        float scaleInLinearGamma = 1.0f;
        float shadowMask = 3.0f;
        float brightBoost = 1.0f;
        float hardBloomPix = -1.5f;
        float hardBloomScan = -2.0f;
        float bloomAmount = 0.15f;
        float shape = 3.0f;

        void InitializeWindow();
        void UpdateShaderVariables(const Shader& shader);
        void DrawImgui();
        Shader SetupShaders();
    };
}
