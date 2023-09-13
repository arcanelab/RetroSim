#pragma once
#include <iostream>
#ifdef LIBRETRO
#include "libretro.h"
#endif

namespace RetroSim::Logger
{
#ifdef LIBRETRO
    extern retro_log_printf_t LogPrintf;

    void SetLibRetroCallback(retro_log_printf_t libRetroPrintf);
#else
    enum retro_log_level
    {
        RETRO_LOG_DEBUG = 0,
        RETRO_LOG_INFO,
        RETRO_LOG_WARN,
        RETRO_LOG_ERROR,

        RETRO_LOG_DUMMY = INT_MAX
    };
    void log_printf(enum retro_log_level level, const char *fmt, ...);

    typedef void (*log_printf_t)(enum retro_log_level level, const char *fmt, ...);
    extern log_printf_t LogPrintf;

#endif

    void Log(const char *message);
    void Log(std::string message);
}; // namespace RetroSim::Logger
