// RetroSim - Copyright 2011-2023 Zoltán Majoros. All rights reserved.
// https://github.com/arcanelab

#pragma once
#include <stdio.h>
#ifdef LIBRETRO
#include "libretro/libretro-common/include/libretro.h"
#endif

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
            stdio, libretro
        };

        Logger(Backend backend)
        {
            this->backend = backend;
        }

        Logger()
        {
        }

        void Log(const char* message)
        {
            switch (backend)
            {   
            case Backend::stdio:
                printf("%s\n", message);
                break;
#ifdef LIBRETRO
            case Backend::libretro:
                libRetroPrintf((retro_log_level)logLevel, "%s\n", message);
                break;
#endif
            default:
                break;
            }
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
        static retro_log_printf_t libRetroPrintf;
#endif
    };

} // namespace RetroSim