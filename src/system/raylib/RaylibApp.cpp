// RetroSim - Copyright 2011-2023 Zoltán Majoros. All rights reserved.
// https://github.com/arcanelab

#include "RaylibApp.h"

#include "imgui.h"
#include "rlImGui.h"

namespace RetroSim
{
        void RaylibApp::Run()
        {
            SetTraceLogLevel(LOG_WARNING);

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
                ClearBackground(BLACK);
                UpdateTexture(drawTexture, GPU::outputTexture);
                BeginShaderMode(shader);
                DrawTextureEx(drawTexture, border, 0.0f, (float)core->GetCoreConfig().GetWindowScale() * desktopScalingFactor, WHITE);
                EndShaderMode();

                rlImGuiBegin();
                // bool open = true;
                // ImGui::ShowDemoWindow(&open);

                DrawImgui();

                rlImGuiEnd();

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
            std::string vertexShaderFileName = "lottes-mini.vs";
            std::string fragmentShaderFileName = "lottes-mini.fs";

            std::string dataPath = core->GetCoreConfig().GetDataPath();
            std::string vertexShaderPath = ConvertPathToPlatformCompatibleFormat(dataPath + "/shaders/" + vertexShaderFileName);
            std::string fragmentShaderPath = ConvertPathToPlatformCompatibleFormat(dataPath + "/shaders/" + fragmentShaderFileName);

            LogPrintf(RETRO_LOG_INFO, "Data path: %s\n", dataPath.c_str());
            LogPrintf(RETRO_LOG_INFO, "Fragment shader path: %s\n", fragmentShaderPath.c_str());
            LogPrintf(RETRO_LOG_INFO, "Vertex shader path: %s\n", vertexShaderPath.c_str());

            Shader shader = LoadShader(vertexShaderPath.c_str(), fragmentShaderPath.c_str());

            Vector2 resolution = {GPU::textureWidth, GPU::textureHeight};

            // Set shader values
            SetShaderValue(shader, GetShaderLocation(shader, "resolution"), &resolution, SHADER_UNIFORM_VEC2);
            UpdateShaderVariables(shader);

            return shader;
        }

        void RaylibApp::UpdateShaderVariables(const Shader& shader)
        {
            SetShaderValue(shader, GetShaderLocation(shader, "LOT_SHARP"), &lotSharp, SHADER_UNIFORM_FLOAT);
            SetShaderValue(shader, GetShaderLocation(shader, "LOT_CURV"), &lotCurv, SHADER_UNIFORM_INT);
            SetShaderValue(shader, GetShaderLocation(shader, "LOT_SCAN"), &lotScan, SHADER_UNIFORM_FLOAT);
            SetShaderValue(shader, GetShaderLocation(shader, "shadowMask"), &shadowMask, SHADER_UNIFORM_INT);
            SetShaderValue(shader, GetShaderLocation(shader, "maskDark"), &maskDark, SHADER_UNIFORM_FLOAT);
            SetShaderValue(shader, GetShaderLocation(shader, "maskLight"), &maskLight, SHADER_UNIFORM_FLOAT);
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
