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

            std::string dataPath = core->GetCoreConfig().GetDataPath();
            std::string fragmentShaderPath = ConvertPathToPlatformCompatibleFormat(dataPath + "/shaders/crt-lotters-fast-fragment.glsl");
            std::string vertexShaderPath = ConvertPathToPlatformCompatibleFormat(dataPath + "/shaders/crt-lotters-fast-vertex.glsl");

            LogPrintf(RETRO_LOG_INFO, "Data path: %s\n", dataPath.c_str());
            LogPrintf(RETRO_LOG_INFO, "Fragment shader path: %s\n", fragmentShaderPath.c_str());
            LogPrintf(RETRO_LOG_INFO, "Vertex shader path: %s\n", vertexShaderPath.c_str());

            InitializeWindow();

            Shader shader = LoadShader(0, TextFormat("resources/shaders/glsl%i/swirl.fs", GLSL_VERSION));
            RenderTexture2D target = LoadRenderTexture(scaledWindowWidth, scaledWindowHeight);

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
                DrawTextureEx(drawTexture, border, 0.0f, (float)core->GetCoreConfig().GetWindowScale() * desktopScalingFactor, WHITE);
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
