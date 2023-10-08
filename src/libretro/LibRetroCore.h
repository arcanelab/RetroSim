// RetroSim - Copyright 2011-2023 Zoltán Majoros. All rights reserved.
// https://github.com/arcanelab

#ifdef LIBRETRO
#pragma once

#include <string>
#include "libretro.h"
#include "Core.h"

namespace RetroSim
{
    // This class is a wrapper around the LibRetro C API callbacks.
    class LibRetroCore
    {
    public:
        void SetEnvironment(retro_environment_t envCallback);
        void Init();
        void SetVideoRefreshCallback(retro_video_refresh_t renderCallback);
        void Run();
        void GetSystemInfo(struct retro_system_info *info);
        void GetSystemAudioVideoInfo(struct retro_system_av_info *info);
        bool LoadGame(const struct retro_game_info *info);
        void UnloadGame();
        static void GenerateAudio();
        static void SetAudioState(bool value);
        void SetAudioSampleCallback(retro_audio_sample_t _audioCallback);
        void SetBatchedAudioCallback(retro_audio_sample_batch_t _batchedAudioCallback);

    private:
        std::string systemDirectory = ".";
        std::string saveDirectory = ".";
        uint32_t *windowBuffer = nullptr;

        Core *coreInstance = nullptr;
        bool scriptingEnabled;

        retro_video_refresh_t renderCallback;
        retro_environment_t envCallback;
        static retro_audio_sample_t audioCallback;
        static retro_audio_sample_batch_t batchedAudioCallback;

        float last_aspect;
        float last_sample_rate;
        bool use_audio_cb;

        static int phase;

        void SetupLogging();
        void SetupControllers();
        void GetSystemDirectory();
        void SetupCore();
        void BlitToRenderBuffer();
    }; // class LibRetroCore
} // namespace RetroSim

#endif