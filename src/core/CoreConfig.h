// RetroSim - Copyright 2011-2023 Zoltán Majoros. All rights reserved.
// https://github.com/arcanelab

#pragma once
#include <string>

namespace RetroSim
{
    class CoreConfig
    {
    public:
        void Initialize(const std::string &basePath);
        void SetScriptPath(const std::string &scriptPath);
        bool IsFullScreen();

        std::string GetDataPath();
        std::string GetScriptPath();        
        int GetFPS();
        void SetTargetFPS(int fps);
        int GetWindowScale();
        int GetAudioSampleRate();

    private:
        std::string basePath = ".";   // All paths are relative to this. Supplied externally via Initialize().
        std::string dataPath = ".";   // This is where the font files are.
        std::string scriptPath = "."; // The path to the script to be run on start.
        bool fullscreen = false;
        int fpsOverride = 0;         // The user can override the default fps.
        int targetFps = 60;          // The current fps. Set to the current freq. of the monitor.
        int audioSampleRate = 48000; // The audio engine will output samples on this frequency.
        int windowScale = 1;         // The window will be scaled by this factor.

        bool isInitialized = false;

        void LoadConfigFile();
    };
}