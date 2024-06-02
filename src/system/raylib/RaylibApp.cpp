// RetroSim - Copyright 2011-2023 Zoltán Majoros. All rights reserved.
// https://github.com/arcanelab

#include "RaylibApp.h"

namespace RetroSim
{
        void RaylibApp::Run()
        {
            char basePath[] = ".";
            core = Core::GetInstance();
            core->Initialize(basePath);

            std::string vertexShaderFileName = "lottes-mini.vs";
            std::string fragmentShaderFileName = "lottes-mini.fs";

            std::string dataPath = core->GetCoreConfig().GetDataPath();
            std::string vertexShaderPath = ConvertPathToPlatformCompatibleFormat(dataPath + "/shaders/" + vertexShaderFileName);
            std::string fragmentShaderPath = ConvertPathToPlatformCompatibleFormat(dataPath + "/shaders/" + fragmentShaderFileName);

            LogPrintf(RETRO_LOG_INFO, "Data path: %s\n", dataPath.c_str());
            LogPrintf(RETRO_LOG_INFO, "Fragment shader path: %s\n", fragmentShaderPath.c_str());
            LogPrintf(RETRO_LOG_INFO, "Vertex shader path: %s\n", vertexShaderPath.c_str());

            InitializeWindow();

            Shader shader = LoadShader(vertexShaderPath.c_str(), fragmentShaderPath.c_str());
            RenderTexture2D target = LoadRenderTexture(scaledWindowWidth, scaledWindowHeight);

            Vector2 resolution = {GPU::textureWidth, GPU::textureHeight};
            float lotSharp = 0.5f;
            float lotCurv = 0.3f;
            float lotScan = 0.3f;
            float shadowMask = 2.0f;
            float maskDark = 0.5f;
            float maskLight = 1.5f;

            // Set shader values
            SetShaderValue(shader, GetShaderLocation(shader, "resolution"), &resolution, SHADER_UNIFORM_VEC2);
            SetShaderValue(shader, GetShaderLocation(shader, "LOT_SHARP"), &lotSharp, SHADER_UNIFORM_FLOAT);
            SetShaderValue(shader, GetShaderLocation(shader, "LOT_CURV"), &lotCurv, SHADER_UNIFORM_FLOAT);
            SetShaderValue(shader, GetShaderLocation(shader, "LOT_SCAN"), &lotScan, SHADER_UNIFORM_FLOAT);
            SetShaderValue(shader, GetShaderLocation(shader, "shadowMask"), &shadowMask, SHADER_UNIFORM_FLOAT);
            SetShaderValue(shader, GetShaderLocation(shader, "maskDark"), &maskDark, SHADER_UNIFORM_FLOAT);
            SetShaderValue(shader, GetShaderLocation(shader, "maskLight"), &maskLight, SHADER_UNIFORM_FLOAT);

            Image drawBuffer = GenImageColor(GPU::textureWidth, GPU::textureHeight, BLANK);
            Texture2D drawTexture = LoadTextureFromImage(drawBuffer);
            UnloadImage(drawBuffer);

            Vector2 border = {(GPU::windowWidth - GPU::textureWidth) * effectiveScalingFactor / 2.0f, (GPU::windowHeight - GPU::textureHeight) * effectiveScalingFactor / 2.0f};

            while (!WindowShouldClose())
            {
                core->RunNextFrame();
                BeginDrawing();
                // ClearBackground(BLACK);
                UpdateTexture(drawTexture, GPU::outputTexture);
                BeginShaderMode(shader);
                DrawTextureEx(drawTexture, border, 0.0f, (float)core->GetCoreConfig().GetWindowScale() * desktopScalingFactor, WHITE);
                EndShaderMode();
                EndDrawing();
            }
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

} // namespace
