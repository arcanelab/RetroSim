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

        const int screenWidth = 480;
        const int screenHeight = 256;

        std::string GetDataPath();
        std::string GetScriptPath();        
        int GetFPS();
        void SetFPS(int fps);

    private:
        std::string basePath = ".";   // All paths are relative to this. Supplied externally via Initialize().
        std::string dataPath = ".";   // This is where the font files are.
        std::string scriptPath = "."; // The path to the script to be run on start.
        bool fullscreen = false;
        int fpsOverride = 0;         // The user can override the default fps.
        int fps = 60;                // The current fps. Set to the current freq. of the monitor.
        int audioSampleRate = 48000; // The audio engine will output samples on this frequency.

        bool isInitialized = false;

        void LoadConfigFile();
    };
}