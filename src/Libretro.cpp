#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include "Core.h"
#include "CoreConfig.h"
#include "GPU.h"
#include "GravityScripting.h"

#include <stdio.h>
#if defined(_WIN32) && !defined(_XBOX)
#include <windows.h>
#endif
#include "libretro/libretro-common/include/libretro.h"

using namespace RetroSim;

static uint8_t *frame_buf;
static struct retro_log_callback logging;
static retro_log_printf_t log_cb;
static bool use_audio_cb;
static float last_aspect;
static float last_sample_rate;
char retro_base_directory[4096];
char retro_game_path[4096];

static retro_video_refresh_t video_cb;
static retro_audio_sample_t audio_cb;
static retro_audio_sample_batch_t audio_batch_cb;
static retro_input_poll_t input_poll_cb;
static retro_input_state_t input_state_cb;

static void fallback_log(enum retro_log_level level, const char *fmt, ...)
{
    (void)level;
    va_list va;
    va_start(va, fmt);
    vfprintf(stderr, fmt, va);
    va_end(va);
}

static retro_environment_t environ_cb;

bool scriptingEnabled;
Core *coreInstance;

void retro_init(void)
{
    const char *dir = NULL;
    if (environ_cb(RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY, &dir) && dir)
    {
#ifdef _WIN32
        _snprintf(retro_base_directory, sizeof(retro_base_directory), "%s\\", dir);
#else
        snprintf(retro_base_directory, sizeof(retro_base_directory), "%s/", dir);
#endif
    }
    // log_cb(RETRO_LOG_INFO, "retro_game_path = %s\n", retro_game_path);

    coreInstance = Core::GetInstance();
    coreInstance->Initialize(retro_base_directory);
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

void retro_deinit(void)
{
    free(frame_buf);
    frame_buf = NULL;
}

static void update_input(void)
{
}

static void check_variables(void)
{
}

void retro_run(void)
{
    update_input();

    bool updated = false;
    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE, &updated) && updated)
        check_variables();

    if (scriptingEnabled)
        GravityScripting::RunScript("update", {}, 0);

    Core::GetInstance()->RunNextFrame();

    video_cb(GPU::outputTexture, GPU::textureWidth, GPU::textureHeight, GPU::textureWidth * sizeof(uint32_t));
}

unsigned retro_api_version(void)
{
    return RETRO_API_VERSION;
}

void retro_set_controller_port_device(unsigned port, unsigned device)
{
    log_cb(RETRO_LOG_INFO, "Plugging device %u into port %u.\n", device, port);
}

void retro_get_system_info(struct retro_system_info *info)
{
    memset(info, 0, sizeof(*info));
    info->library_name = "RetroSim";
    info->library_version = "0.1";
    info->need_fullpath = true;
    info->valid_extensions = "rsx";
    info->block_extract = false;
}

void retro_get_system_av_info(struct retro_system_av_info *info)
{
    float aspect = 0; // zero defaults to width/height
    float sampling_rate = 48000.0f;

    info->geometry.base_width = GPU::textureWidth;
    info->geometry.base_height = GPU::textureHeight;
    info->geometry.max_width = GPU::textureWidth;
    info->geometry.max_height = GPU::textureHeight;
    info->geometry.aspect_ratio = 0;
    info->timing.fps = 30;

    last_aspect = aspect;
    last_sample_rate = sampling_rate;

    // TODO: check where the pixel format should be set at
    // retro_pixel_format pixel_format = RETRO_PIXEL_FORMAT_XRGB8888;
    // environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &pixel_format);
}

void retro_set_environment(retro_environment_t cb)
{
    environ_cb = cb;

    if (cb(RETRO_ENVIRONMENT_GET_LOG_INTERFACE, &logging))
        log_cb = logging.log;
    else
        log_cb = fallback_log;

    static const struct retro_controller_description controllers[] = {
        {"Nintendo DS", RETRO_DEVICE_SUBCLASS(RETRO_DEVICE_JOYPAD, 0)},
    };

    static const struct retro_controller_info ports[] = {
        {controllers, 1},
        {NULL, 0},
    };

    cb(RETRO_ENVIRONMENT_SET_CONTROLLER_INFO, (void *)ports);
}

void retro_set_audio_sample(retro_audio_sample_t cb)
{
    audio_cb = cb;
}

void retro_set_audio_sample_batch(retro_audio_sample_batch_t cb)
{
    audio_batch_cb = cb;
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
    video_cb = cb;
}

void retro_reset(void)
{
    log_cb(RETRO_LOG_INFO, "retro_reset()\n");
    retro_init();
}

static void audio_callback(void)
{
    // test sine wave
    // for (unsigned i = 0; i < 48000 / 60; i++, phase++)
    // {
    //     int16_t val = 0x800 * sinf(2.0f * 3.14159265f * phase * 300.0f / 48000.0f);
    //     audio_cb(val, val);
    // }

    // phase %= 100;
}

static void audio_set_state(bool enable)
{
    (void)enable;
}

bool retro_load_game(const struct retro_game_info *info)
{
    log_cb(RETRO_LOG_INFO, "retro_load_game()\n");

    struct retro_input_descriptor desc[] = {
        {0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT, "Left"},
        {0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP, "Up"},
        {0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN, "Down"},
        {0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT, "Right"},
        {0},
    };

    environ_cb(RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS, desc);

    // Note: is this the right place for these environ callbacks?
    enum retro_pixel_format fmt = RETRO_PIXEL_FORMAT_XRGB8888;
    if (!environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &fmt))
    {
        log_cb(RETRO_LOG_INFO, "XRGB8888 is not supported.\n");
        return false;
    }

    struct retro_audio_callback audio_cb = {audio_callback, audio_set_state};
    use_audio_cb = environ_cb(RETRO_ENVIRONMENT_SET_AUDIO_CALLBACK, &audio_cb);

    check_variables();

    (void)info;
    return true;
}

void retro_unload_game(void)
{
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
