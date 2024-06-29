// RetroSim - Copyright 2011-2023 Zoltán Majoros. All rights reserved.
// https://github.com/arcanelab

#include "RaylibApp.h"
#ifdef IMGUI
#include "imgui.h"
#include "rlImGui.h"
#endif // IMGUI

namespace RetroSim
{
    void RaylibApp::Run()
    {
        SetTraceLogLevel(LOG_ERROR);

        char basePath[] = ".";
        core = Core::GetInstance();
        core->Initialize(basePath);

        InitializeWindow();

#ifdef IMGUI
        rlImGuiSetup(true);
#endif // IMGUI

        shader.Initialize(core, scaledWindowWidth, scaledWindowHeight);

        Image drawBuffer = GenImageColor(GPU::textureWidth, GPU::textureHeight, BLANK);
        Texture2D drawTexture = LoadTextureFromImage(drawBuffer);
        UnloadImage(drawBuffer);

        Vector2 border = {(GPU::windowWidth - GPU::textureWidth) * effectiveScalingFactor / 2.0f, (GPU::windowHeight - GPU::textureHeight) * effectiveScalingFactor / 2.0f};

        while (!WindowShouldClose())
        {
            if(core->IsPaused() == false)
                core->RunNextFrame();

            shader.UpdateShaderVariables();
            BeginDrawing();
            {
                ClearBackground(BLANK);
                UpdateTexture(drawTexture, GPU::outputTexture);
                BeginShaderMode(shader.GetShader());
                {
                    DrawTextureEx(drawTexture, border, 0.0f, (float)core->GetCoreConfig().GetWindowScale() * desktopScalingFactor, WHITE);
                }
                EndShaderMode();
#ifdef IMGUI
                ImGuiIO& io = ImGui::GetIO();
                io.FontGlobalScale = core->GetCoreConfig().GetWindowScale();

                rlImGuiBegin();
                bool open = true;
                ImGui::ShowDemoWindow(&open);
                shader.DrawParametersGui();
                core->DrawImGui();
                rlImGuiEnd();
#endif // IMGUI
            }
            EndDrawing();
        }

#ifdef IMGUI
        rlImGuiShutdown();
#endif // IMGUI
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
