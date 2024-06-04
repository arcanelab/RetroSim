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

        shaderManager.SetupShaders(core, scaledWindowWidth, scaledWindowHeight);

        Image drawBuffer = GenImageColor(GPU::textureWidth, GPU::textureHeight, BLANK);
        Texture2D drawTexture = LoadTextureFromImage(drawBuffer);
        UnloadImage(drawBuffer);

        Vector2 border = {(GPU::windowWidth - GPU::textureWidth) * effectiveScalingFactor / 2.0f, (GPU::windowHeight - GPU::textureHeight) * effectiveScalingFactor / 2.0f};

        while (!WindowShouldClose())
        {
            core->RunNextFrame();

            shaderManager.UpdateShaderVariables();
            BeginDrawing();
            {
                ClearBackground(BLANK);
                UpdateTexture(drawTexture, GPU::outputTexture);
                BeginShaderMode(shaderManager.GetShader());
                {
                    DrawTextureEx(drawTexture, border, 0.0f, (float)core->GetCoreConfig().GetWindowScale() * desktopScalingFactor, WHITE);
                }
                EndShaderMode();
                rlImGuiBegin();
                // bool open = true;
                // ImGui::ShowDemoWindow(&open);
                shaderManager.DrawParametersGui();
                rlImGuiEnd();
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

} // namespace
