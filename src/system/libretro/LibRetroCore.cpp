// RetroSim - Copyright 2011-2023 Zoltán Majoros. All rights reserved.
// https://github.com/arcanelab

#ifdef LIBRETRO

#include "LibRetroCore.h"
#include "GPU.h"
#include "MMU.h"
#include "Logger.h"
#include "FileUtils.h"

namespace RetroSim
{
    retro_audio_sample_t LibRetroCore::audioCallback = nullptr;
    retro_audio_sample_batch_t LibRetroCore::batchedAudioCallback = nullptr;

    void LibRetroCore::SetEnvironment(retro_environment_t envCallback)
    {
        this->envCallback = envCallback;

        SetupLogging();
        // SetupControllers();
        // GetSystemDirectory();

        // Communicate to the frontend that we don't require a game before running the core.
        bool noGameSupport = true;
        envCallback(RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME, &noGameSupport);
    }

    void LibRetroCore::Init()
    {
        // GetSystemDirectory();
        SetupCore();

        if (windowBuffer == nullptr)
            windowBuffer = new uint32_t[GPU::windowWidth * GPU::windowHeight];

        memset(windowBuffer, 0, GPU::windowWidth * GPU::windowHeight * sizeof(uint32_t));
    }

    void LibRetroCore::SetVideoRefreshCallback(retro_video_refresh_t renderCallback)
    {
        this->renderCallback = renderCallback;
    }

    void LibRetroCore::BlitToRenderBuffer()
    {
        uint32_t *dst = (uint32_t *)windowBuffer + GPU::windowWidth * (GPU::windowHeight - GPU::textureHeight) / 2 + (GPU::windowWidth - GPU::textureWidth) / 2;
        uint32_t *src = (uint32_t *)GPU::outputTexture;
        for (uint_fast16_t y = 0; y < GPU::textureHeight; y++)
        {
            for (uint_fast16_t x = 0; x < GPU::textureWidth; x++)
            {
                uint32_t pixel = *src++;
                uint8_t a = (pixel >> 24) & 0xff;
                uint8_t r = (pixel >> 16) & 0xff;
                uint8_t g = (pixel >> 8) & 0xff;
                uint8_t b = (pixel >> 0) & 0xff;
                *dst++ = (a << 24) | (b << 16) | (g << 8) | (r << 0);
            }

            dst += GPU::windowWidth - GPU::textureWidth;
        }
    }

    void LibRetroCore::Run()
    {
        // TODO: check for input
        // TODO: check for variable changes via RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE

        Core::GetInstance()->RunNextFrame();

        BlitToRenderBuffer();
        renderCallback(windowBuffer, GPU::windowWidth, GPU::windowHeight, GPU::windowWidth * sizeof(uint32_t));
    }

    void LibRetroCore::GetSystemInfo(struct retro_system_info *info)
    {
        memset(info, 0, sizeof(*info));
        info->library_name = "RetroSim";
        info->library_version = "0.1";
        info->need_fullpath = true;
        info->valid_extensions = "rsx";
        info->block_extract = false;
    }

    void LibRetroCore::GetSystemAudioVideoInfo(struct retro_system_av_info *info)
    {
        float aspect = 0; // zero defaults to width/height
        float sampling_rate = (float)Core::GetInstance()->GetSampleRate();

        info->geometry.base_width = GPU::windowWidth;
        info->geometry.base_height = GPU::windowHeight;
        info->geometry.max_width = GPU::windowWidth;
        info->geometry.max_height = GPU::windowHeight;
        info->geometry.aspect_ratio = 0;
        info->timing.fps = coreInstance->GetCoreConfig().GetFPS();
        info->timing.sample_rate = sampling_rate;

        static float last_aspect = aspect;
        static float last_sample_rate = sampling_rate;

        // Note: this might not even be necessary as the core runs fine without this call.
        // Leaving it here for good measure.
        retro_pixel_format pixel_format = RETRO_PIXEL_FORMAT_XRGB8888;
        envCallback(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &pixel_format);
    }

    bool LibRetroCore::LoadGame(const struct retro_game_info *info)
    {
        Logger::LogPrintf(RETRO_LOG_INFO, "retro_load_game()");

        struct retro_input_descriptor desc[] = {
            {0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT, "Left"},
            {0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP, "Up"},
            {0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN, "Down"},
            {0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT, "Right"},
            {0},
        };

        envCallback(RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS, desc);

        // Note: This can be called either here or in GetSystemAudioVideoInfo().
        enum retro_pixel_format fmt = RETRO_PIXEL_FORMAT_XRGB8888;
        if (!envCallback(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &fmt))
        {
            Logger::LogPrintf(RETRO_LOG_INFO, "XRGB8888 is not supported.\n");
            return false;
        }

        struct retro_audio_callback audio_cb = {GenerateAudio, SetAudioState};
        use_audio_cb = envCallback(RETRO_ENVIRONMENT_SET_AUDIO_CALLBACK, &audio_cb);

        // check_variables();

        (void)info;
        return true;
    }

    void LibRetroCore::UnloadGame()
    {
        Logger::LogPrintf(RETRO_LOG_INFO, "retro_unload_game()\n");
    }

    void LibRetroCore::GenerateAudio()
    {
        // test sine wave
        // static int phase;
        // for (unsigned i = 0; i < 48000 / 60; i++, phase++)
        // {
        //     int16_t val = 0x800 * sinf(2.0f * 3.14159265f * phase * 300.0f / 48000.0f);
        //     audioCallback(val, val);
        // }
        // phase %= 100;

        Core *core = Core::GetInstance();
        uint16_t *audioBuffer = nullptr;
        uint32_t audioBufferSize = 0;
        core->RenderAudio(&audioBuffer, &audioBufferSize);
        batchedAudioCallback((const int16_t *)audioBuffer, audioBufferSize / 2);
    }

    void LibRetroCore::SetAudioState(bool value)
    {
        (void)value;
    }

    void LibRetroCore::SetAudioSampleCallback(retro_audio_sample_t _audioCallback)
    {
        LibRetroCore::audioCallback = _audioCallback;
    }

    void LibRetroCore::SetBatchedAudioCallback(retro_audio_sample_batch_t _batchedAudioCallback)
    {
        LibRetroCore::batchedAudioCallback = _batchedAudioCallback;
    }

    // We try gettig a callback from the frontend and set it as a backend in our Logger class.
    // If we fail, the Logger class falls back to stdio.
    void LibRetroCore::SetupLogging()
    {
        retro_log_callback logCallback;

        if (envCallback(RETRO_ENVIRONMENT_GET_LOG_INTERFACE, &logCallback))
        {
            Logger::SetLibRetroCallback(logCallback.log);
            Logger::LogPrintf(RETRO_LOG_DEBUG, "Logging initialized.\n");
        }
    }

    // We define a single, "joypad" style controller as set it as the only controller.
    // Todo: Add support for multiple controllers, suitable for the current build target.
    void LibRetroCore::SetupControllers()
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

    void LibRetroCore::GetSystemDirectory()
    {
        const char *dir = NULL;
        if (envCallback(RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY, &dir) && dir)
        {
#ifdef WIN32
            systemDirectory = RetroSim::ConvertPathToPlatformCompatibleFormat(dir);
#else
            systemDirectory = dir;
#endif

            Logger::LogPrintf(RETRO_LOG_DEBUG, "System directory: %s\n", systemDirectory.c_str());
        }
        else
        {
            Logger::LogPrintf(RETRO_LOG_ERROR, "Failed to get system directory.\n");
        }
    }

    void LibRetroCore::SetupCore()
    {
        GetSystemDirectory();
        coreInstance = Core::GetInstance();
        coreInstance->Initialize(systemDirectory.c_str());
        // CoreConfig config = coreInstance->GetCoreConfig();
    }
} // namespace RetroSim

#endif