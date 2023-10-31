// #include "RaylibApp.h"
#include "raylib.h"
#include "Core.h"
#include "GPU.h"
#include "Logger.h"
#include "FileUtils.h"

#if defined(PLATFORM_DESKTOP)
#define GLSL_VERSION 330
#else // PLATFORM_ANDROID, PLATFORM_WEB
#define GLSL_VERSION 100
#endif

namespace RetroSim
{
    class RaylibApp
    {
    public:
        void Run()
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

            while (!WindowShouldClose())
            {
                core->RunNextFrame();
                BeginDrawing();
                ClearBackground(RAYWHITE);
                DrawText("RetroSim v0.2.0 (Raylib)", 190, 200, 20, LIGHTGRAY);
                EndDrawing();
            }
            CloseWindow();
        }

        void InitializeWindow()
        {
            int windowScalingFactor = core->GetCoreConfig().GetWindowScale();

            scaledWindowWidth = GPU::windowWidth * windowScalingFactor;
            scaledWindowHeight = GPU::windowHeight * windowScalingFactor;

            InitWindow(scaledWindowWidth, scaledWindowHeight, "RetroSim");

            int currentMonitor = GetCurrentMonitor();
            int refreshRate = GetMonitorRefreshRate(currentMonitor);
            LogPrintf(RETRO_LOG_INFO, "Refresh rate: %d\n", refreshRate);

            Core::GetInstance()->SetRefreshRate(refreshRate);

            SetTargetFPS(refreshRate);
        }

    private:
        Core *core;
        int scaledWindowWidth;
        int scaledWindowHeight;
    };

}