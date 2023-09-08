// RetroSim - Copyright 2011-2023 Zoltán Majoros. All rights reserved.
// https://github.com/arcanelab

#pragma once
#include <stdio.h>
#ifdef LIBRETRO
#include "libretro/libretro-common/include/libretro.h"
#endif
#include "CoreConfig.h"

namespace RetroSim
{
    class Logger
    {
    public:
        enum LogLevel
        {
            LOG_DEBUG = 0,
            LOG_INFO,
            LOG_WARN,
            LOG_ERROR,
        };

        enum Backend
        {
            stdio,
            libretro
        };

        Logger(Backend backend)
        {
            this->backend = backend;
        }

        Logger()
        {
        }

        ~Logger()
        {
        }

#ifdef LIBRETRO
        void Log(enum retro_log_level level, const char *fmt, ...)
        {
            va_list va;
            va_start(va, fmt);
            libRetroPrintf((retro_log_level)logLevel, fmt, va);
            va_end(va);
        }
#endif

        void Log(const char *message)
        {
            printf("%s\n", message);
        }

        void SetBackend(Backend backend)
        {
            Logger::backend = backend;
        }

        void SetLogLevel(LogLevel logLevel)
        {
            Logger::logLevel = logLevel;
        }

#ifdef LIBRETRO
        void SetLibRetroCallback(retro_log_printf_t libRetroPrintf)
        {
            this->libRetroPrintf = libRetroPrintf;
        }
#endif

    private:
        Backend backend = Backend::stdio;
        LogLevel logLevel = LogLevel::LOG_DEBUG;

#ifdef LIBRETRO
        retro_log_printf_t libRetroPrintf;
#endif
    };

} // namespace RetroSim