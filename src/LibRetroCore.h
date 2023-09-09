// RetroSim - Copyright 2011-2023 Zoltán Majoros. All rights reserved.
// https://github.com/arcanelab

#pragma once
#ifdef LIBRETRO

#include <string>
#include "libretro/libretro-common/include/libretro.h"
#include "Logger.h"
#include "Core.h"
#include "GravityScripting.h"
#include "GPU.h"

// TODO: test if this is needed on Windows.
#if defined(_WIN32) && !defined(_XBOX)
#include <windows.h>
#endif

namespace RetroSim
{
    class LibRetroCore
    {
    public:
        void SetEnvironment(retro_environment_t envCallback)
        {
            this->envCallback = envCallback;

            SetupLogging();
            SetupControllers();
            GetSystemDirectory();

            // Communicate to the frontend that we don't require a game before running the core.
            bool noGameSupport = true;
            envCallback(RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME, &noGameSupport);
        }

        void Init()
        {
            // GetSystemDirectory();
            SetupCore();

            CoreConfig config = coreInstance->GetCoreConfig();
            scriptingEnabled = !config.GetScriptPath().empty();
            if (scriptingEnabled)
            {
                printf("Running script: %s\n", config.GetScriptPath().c_str());
                GravityScripting::RegisterAPIFunctions();
                GravityScripting::CompileScriptFromFile(config.GetScriptPath());
                GravityScripting::RunScript("start", {}, 0);
            }
        }

        void SetVideoRefreshCallback(retro_video_refresh_t renderCallback)
        {
            this->renderCallback = renderCallback;
        }

        void Run()
        {
            // TODO: check for input
            // TODO: check for variable changes via RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE

            if (scriptingEnabled)
                GravityScripting::RunScript("update", {}, 0);

            Core::GetInstance()->RunNextFrame();

            renderCallback(GPU::outputTexture, GPU::textureWidth, GPU::textureHeight, GPU::textureWidth * sizeof(uint32_t));
        }

        void GetSystemInfo(struct retro_system_info *info)
        {
            memset(info, 0, sizeof(*info));
            info->library_name = "RetroSim";
            info->library_version = "0.1";
            info->need_fullpath = true;
            info->valid_extensions = "rsx";
            info->block_extract = false;
        }

        void GetSystemAudioVideoInfo(struct retro_system_av_info *info)
        {
            float aspect = 0; // zero defaults to width/height
            float sampling_rate = 48000.0f;

            info->geometry.base_width = GPU::textureWidth;
            info->geometry.base_height = GPU::textureHeight;
            info->geometry.max_width = GPU::textureWidth;
            info->geometry.max_height = GPU::textureHeight;
            info->geometry.aspect_ratio = 0;
            info->timing.fps = coreInstance->GetCoreConfig().GetFPS();

            last_aspect = aspect;
            last_sample_rate = sampling_rate;

            // Note: this might not even be necessary as the core runs fine without this call.
            // Leaving it here for good measure.
            retro_pixel_format pixel_format = RETRO_PIXEL_FORMAT_XRGB8888;
            envCallback(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &pixel_format);
        }

        bool LoadGame(const struct retro_game_info *info)
        {
            // log_cb(RETRO_LOG_INFO, "retro_load_game()\n");
            logger.Printf(RETRO_LOG_INFO, "retro_load_game()");

            struct retro_input_descriptor desc[] = {
                {0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT, "Left"},
                {0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP, "Up"},
                {0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN, "Down"},
                {0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT, "Right"},
                {0},
            };

            envCallback(RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS, desc);

            // Note: This can be called eirther here or in GetSystemAudioVideoInfo().
            enum retro_pixel_format fmt = RETRO_PIXEL_FORMAT_XRGB8888;
            if (!envCallback(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &fmt))
            {
                logger.Printf(RETRO_LOG_INFO, "XRGB8888 is not supported.\n");
                return false;
            }

            // struct retro_audio_callback audio_cb = {audio_callback, audio_set_state};
            struct retro_audio_callback audio_cb = {GenerateAudio, SetAudioState};
            use_audio_cb = envCallback(RETRO_ENVIRONMENT_SET_AUDIO_CALLBACK, &audio_cb);

            // check_variables();

            (void)info;
            return true;
        }

        void UnloadGame()
        {
            logger.Printf(RETRO_LOG_INFO, "retro_unload_game()\n");            
        }

        static void GenerateAudio()
        {
            // test sine wave
            for (unsigned i = 0; i < 48000 / 60; i++, phase++)
            {
                int16_t val = 0x800 * sinf(2.0f * 3.14159265f * phase * 300.0f / 48000.0f);
                audioCallback(val, val);
            }

            phase %= 100;
        }

        static void SetAudioState(bool value)
        {
            (void)value;
        }

        void SetAudioSampleCallback(retro_audio_sample_t _audioCallback)
        {
            LibRetroCore::audioCallback = _audioCallback;
        }

        void SetBatchedAudioCallback(retro_audio_sample_batch_t _batchedAudioCallback)
        {
            LibRetroCore::batchedAudioCallback = _batchedAudioCallback;
        }

        Logger logger;

    private:
        std::string systemDirectory = ".";
        std::string saveDirectory = ".";

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

        // We try gettig a callback from the frontend and set it as a backend in our Logger class.
        // If we fail, the Logger class falls back to stdio.
        void SetupLogging()
        {
            retro_log_callback logCallback;

            if (envCallback(RETRO_ENVIRONMENT_GET_LOG_INTERFACE, &logCallback))
            {
                logger.SetLibRetroCallback(logCallback.log);
            }
        }

        // We define a single, "joypad" style controller as set it as the only controller.
        // Todo: Add support for multiple controllers, suitable for the current build target.
        void SetupControllers()
        {
            static const struct retro_controller_description controllers[] =
                {
                    {"Nintendo DS", RETRO_DEVICE_SUBCLASS(RETRO_DEVICE_JOYPAD, 0)},
                };

            static const struct retro_controller_info ports[] =
                {
                    {controllers, 1},
                    {NULL, 0},
                };

            envCallback(RETRO_ENVIRONMENT_SET_CONTROLLER_INFO, (void *)ports);
        }

        void GetSystemDirectory()
        {
            const char *dir = NULL;
            if (envCallback(RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY, &dir) && dir)
            {
                systemDirectory = dir;
                logger.Printf(RETRO_LOG_DEBUG, "System directory: %s\n", systemDirectory.c_str());
            }
            else
            {
                logger.Printf(RETRO_LOG_ERROR, "Failed to get system directory.\n");
            }
        }

        void SetupCore()
        {
            coreInstance = Core::GetInstance();
            coreInstance->Initialize(systemDirectory.c_str());
            // CoreConfig config = coreInstance->GetCoreConfig();
        }
    }; // class LibRetroCore

    retro_audio_sample_t LibRetroCore::audioCallback = nullptr;
    retro_audio_sample_batch_t LibRetroCore::batchedAudioCallback = nullptr;
    int LibRetroCore::phase = 0;
} // namespace RetroSim

#endif