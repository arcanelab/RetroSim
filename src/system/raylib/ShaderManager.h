#pragma once
#include "raylib.h"
#include "Core.h"

namespace RetroSim::Raylib
{
    struct Parameters
    {
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
    };

    class ShaderManager
    {
    public:
        Parameters parameters;
        Shader GetShader() { return shader; };
        void UpdateShaderVariables();
        void SetupShaders(RetroSim::Core *core, const int scaledWindowWidth, const int scaledWindowHeight);
        void DrawParametersGui();
    private:
        Shader shader;
    };

}
