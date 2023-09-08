// RetroSim - Copyright 2011-2023 Zoltán Majoros. All rights reserved.
// https://github.com/arcanelab

#pragma once
#ifdef LIBRETRO
#include <string>
#include "libretro/libretro-common/include/libretro.h"
#include "Logger.h"

namespace RetroSim
{
    class LibRetroCore
    {
        std::string systemDirectory;
        std::string saveDirectory;

        retro_environment_t envCallback;
        Logger logger;

        void SetEnvironment(retro_environment_t envCallback)
        {
            SetupLogging();
        }

        private:

        // We try gettig a callback from the frontend and set it as a backend in our Logger class.
        // If we fail, the Logger class falls back to stdio.
        void SetupLogging()
        {
            logger.SetBackend(Logger::Backend::libretro);

            retro_log_callback logCallback;
            retro_log_printf_t logPrintf;

            this->envCallback = envCallback;

            if(envCallback(RETRO_ENVIRONMENT_GET_LOG_INTERFACE, &logCallback))
            {
                logger.SetLibRetroCallback(logCallback.log);
            }
        }
    };
}
#endif