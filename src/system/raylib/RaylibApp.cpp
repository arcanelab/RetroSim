// RetroSim - Copyright 2011-2023 Zoltán Majoros. All rights reserved.
// https://github.com/arcanelab

#include "RaylibApp.h"

#include "imgui.h"
#include "rlImGui.h"

namespace RetroSim
{
    void RaylibApp::Run()
    {
        SetTraceLogLevel(LOG_ERROR);

        char basePath[] = ".";
        core = Core::GetInstance();
        core->Initialize(basePath);

        InitializeWindow();
        rlImGuiSetup(true);
        auto imguiio = ImGui::GetIO();
        imguiio.FontGlobalScale = 2.0f;

        Shader shader = SetupShaders();

        Image drawBuffer = GenImageColor(GPU::textureWidth, GPU::textureHeight, BLANK);
        Texture2D drawTexture = LoadTextureFromImage(drawBuffer);
        UnloadImage(drawBuffer);

        Vector2 border = {(GPU::windowWidth - GPU::textureWidth) * effectiveScalingFactor / 2.0f, (GPU::windowHeight - GPU::textureHeight) * effectiveScalingFactor / 2.0f};

        while (!WindowShouldClose())
        {
            core->RunNextFrame();

            UpdateShaderVariables(shader);
            BeginDrawing();
            {
                ClearBackground(BLANK);
                UpdateTexture(drawTexture, GPU::outputTexture);
                BeginShaderMode(shader);
                {
                    DrawTextureEx(drawTexture, border, 0.0f, (float)core->GetCoreConfig().GetWindowScale() * desktopScalingFactor, WHITE);
                }
                EndShaderMode();
                // rlImGuiBegin();
                // bool open = true;
                // ImGui::ShowDemoWindow(&open);
                // DrawImgui();
                // rlImGuiEnd();
            }
            EndDrawing();
        }

        rlImGuiShutdown();
        CloseWindow();
    }

    void RaylibApp::InitializeWindow()
    {
        windowScalingFactor = core->GetCoreConfig().GetWindowScale();
        desktopScalingFactor = GetDesktopScalingFactor();
        effectiveScalingFactor = windowScalingFactor * desktopScalingFactor;

        LogPrintf(RETRO_LOG_INFO, "Window scaling factor: %d\n", windowScalingFactor);
        LogPrintf(RETRO_LOG_INFO, "Desktop scaling factor: %f\n", desktopScalingFactor);

        scaledWindowWidth = GPU::windowWidth * windowScalingFactor;
        scaledWindowHeight = GPU::windowHeight * windowScalingFactor;

        InitWindow(int(scaledWindowWidth * desktopScalingFactor), int(scaledWindowHeight * desktopScalingFactor), "RetroSim");

        int currentMonitor = GetCurrentMonitor();
        int refreshRate = GetMonitorRefreshRate(currentMonitor);
        LogPrintf(RETRO_LOG_INFO, "Refresh rate: %d\n", refreshRate);

        Core::GetInstance()->SetRefreshRate(refreshRate);

        SetTargetFPS(refreshRate);
    }

    Shader RaylibApp::SetupShaders()
    {
        std::string vertexShaderFileName = "lottes.vs";
        std::string fragmentShaderFileName = "lottes.fs";

        std::string dataPath = core->GetCoreConfig().GetDataPath();
        std::string vertexShaderPath = ConvertPathToPlatformCompatibleFormat(dataPath + "/shaders/" + vertexShaderFileName);
        std::string fragmentShaderPath = ConvertPathToPlatformCompatibleFormat(dataPath + "/shaders/" + fragmentShaderFileName);

        LogPrintf(RETRO_LOG_INFO, "Data path: %s\n", dataPath.c_str());
        LogPrintf(RETRO_LOG_INFO, "Fragment shader path: %s\n", fragmentShaderPath.c_str());
        LogPrintf(RETRO_LOG_INFO, "Vertex shader path: %s\n", vertexShaderPath.c_str());

        Shader shader = LoadShader(vertexShaderPath.c_str(), fragmentShaderPath.c_str());

        Vector2 textureSize = {GPU::textureWidth, GPU::textureHeight};
        Vector2 outputSize = {scaledWindowWidth, scaledWindowHeight};
        Vector2 inputSize = {scaledWindowWidth, scaledWindowHeight};

        // Set shader values
        SetShaderValue(shader, GetShaderLocation(shader, "TextureSize"), &textureSize, SHADER_UNIFORM_VEC2);
        SetShaderValue(shader, GetShaderLocation(shader, "InputSize"), &textureSize, SHADER_UNIFORM_VEC2);
        SetShaderValue(shader, GetShaderLocation(shader, "OutputSize"), &outputSize, SHADER_UNIFORM_VEC2);
        UpdateShaderVariables(shader);

        return shader;
    }

    void RaylibApp::UpdateShaderVariables(const Shader &shader)
    {
        float hardScan;
        float hardPix;
        float warpX;
        float warpY;
        float maskDark;
        float maskLight;
        float scaleInLinearGamma;
        float shadowMask;
        float brightBoost;
        float hardBloomPix;
        float hardBloomScan;
        float bloomAmount;
        float shape;

        SetShaderValue(shader, GetShaderLocation(shader, "hardScan"), &hardScan, SHADER_ATTRIB_FLOAT);
        SetShaderValue(shader, GetShaderLocation(shader, "hardPix"), &hardPix, SHADER_ATTRIB_FLOAT);
        SetShaderValue(shader, GetShaderLocation(shader, "warpX"), &warpX, SHADER_ATTRIB_FLOAT);
        SetShaderValue(shader, GetShaderLocation(shader, "warpY"), &warpY, SHADER_ATTRIB_FLOAT);
        SetShaderValue(shader, GetShaderLocation(shader, "maskDark"), &maskDark, SHADER_ATTRIB_FLOAT);
        SetShaderValue(shader, GetShaderLocation(shader, "maskLight"), &maskLight, SHADER_ATTRIB_FLOAT);
        SetShaderValue(shader, GetShaderLocation(shader, "scaleInLinearGamma"), &scaleInLinearGamma, SHADER_ATTRIB_FLOAT);
        SetShaderValue(shader, GetShaderLocation(shader, "shadowMask"), &shadowMask, SHADER_ATTRIB_FLOAT);
        SetShaderValue(shader, GetShaderLocation(shader, "brightBoost"), &brightBoost, SHADER_ATTRIB_FLOAT);
        SetShaderValue(shader, GetShaderLocation(shader, "hardBloomPix"), &hardBloomPix, SHADER_ATTRIB_FLOAT);
        SetShaderValue(shader, GetShaderLocation(shader, "hardBloomScan"), &hardBloomScan, SHADER_ATTRIB_FLOAT);
        SetShaderValue(shader, GetShaderLocation(shader, "bloomAmount"), &bloomAmount, SHADER_ATTRIB_FLOAT);
        SetShaderValue(shader, GetShaderLocation(shader, "shape"), &shape, SHADER_ATTRIB_FLOAT);
    }

    void RaylibApp::DrawImgui()
    {
        bool isShaderGuiActive = true;
        ImGui::Begin("Shader parameters", &isShaderGuiActive);
        ImGui::SliderFloat("Sharpness", &lotSharp, 0, 3.0f);
        ImGui::SliderInt("Curvature", &lotCurv, 0, 5);
        ImGui::SliderFloat("Scanlines", &lotScan, 0.0f, 1.0f);
        ImGui::SliderInt("Shadow Mask", &shadowMask, 0, 4);
        ImGui::SliderFloat("Mask Dark", &maskDark, 0.0f, 3.0f);
        ImGui::SliderFloat("Mask Light", &maskLight, 0.0f, 3.0f);
        ImGui::End();
    }

} // namespace
