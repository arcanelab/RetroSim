// RetroSim - Copyright 2011-2023 Zoltán Majoros. All rights reserved.
// https://github.com/arcanelab

#ifdef LIBRETRO
#include "LibRetroCore.h"

// LibRetroCore is a wrapper around the LibRetro C API callbacks.
// We forward the callbacks to the LibRetroCore instance.
RetroSim::LibRetroCore libretroCore;

// TODO: implement all callbacks
static retro_input_poll_t input_poll_cb;
static retro_input_state_t input_state_cb;

void retro_set_environment(retro_environment_t cb)
{
    libretroCore.SetEnvironment(cb);
}

void retro_init(void)
{
    libretroCore.Init();
}

void retro_deinit(void)
{
}

static void update_input(void)
{
}

static void check_variables(void)
{
}

void retro_run(void)
{
    libretroCore.Run();
}

unsigned retro_api_version(void)
{
    return RETRO_API_VERSION;
}

void retro_set_controller_port_device(unsigned port, unsigned device)
{
    libretroCore.logger.Printf(RETRO_LOG_INFO, "Plugging device %u into port %u.\n", device, port);
}

void retro_get_system_info(struct retro_system_info *info)
{
    libretroCore.GetSystemInfo(info);
}

void retro_get_system_av_info(struct retro_system_av_info *info)
{
    libretroCore.GetSystemAudioVideoInfo(info);
}

void retro_set_audio_sample(retro_audio_sample_t cb)
{
    libretroCore.SetAudioSampleCallback(cb);
}

void retro_set_audio_sample_batch(retro_audio_sample_batch_t cb)
{
    libretroCore.SetBatchedAudioCallback(cb);
}

void retro_set_input_poll(retro_input_poll_t cb)
{
    input_poll_cb = cb;
}

void retro_set_input_state(retro_input_state_t cb)
{
    input_state_cb = cb;
}

void retro_set_video_refresh(retro_video_refresh_t cb)
{
    libretroCore.SetVideoRefreshCallback(cb);
}

void retro_reset(void)
{
    libretroCore.logger.Printf(RETRO_LOG_INFO, "retro_reset()\n");
    retro_init();
}

static void audio_set_state(bool enable)
{
    libretroCore.SetAudioState(enable);
}

bool retro_load_game(const struct retro_game_info *info)
{
    return libretroCore.LoadGame(info);
}

void retro_unload_game(void)
{
    libretroCore.UnloadGame();
}

unsigned retro_get_region(void)
{
    return RETRO_REGION_NTSC;
}

bool retro_load_game_special(unsigned type, const struct retro_game_info *info, size_t num)
{
    return false;
}

size_t retro_serialize_size(void)
{
    return 0;
}

bool retro_serialize(void *data_, size_t size)
{
    return false;
}

bool retro_unserialize(const void *data_, size_t size)
{
    return false;
}

void *retro_get_memory_data(unsigned id)
{
    (void)id;
    return NULL;
}

size_t retro_get_memory_size(unsigned id)
{
    (void)id;
    return 0;
}

void retro_cheat_reset(void)
{
}

void retro_cheat_set(unsigned index, bool enabled, const char *code)
{
    (void)index;
    (void)enabled;
    (void)code;
}

#endif