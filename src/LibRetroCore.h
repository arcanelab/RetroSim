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
    public:
        void SetEnvironment(retro_environment_t envCallback)
        {
            this->envCallback = envCallback;

            SetupLogging();
            SetupControllers();
            // GetSystemDirectory();

            // Communicate to the frontend that we don't require a game before running the core.
            bool noGameSupport = true;
            envCallback(RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME, &noGameSupport);
        }

        Logger logger;

    private:
        std::string systemDirectory = ".";
        std::string saveDirectory = ".";

        retro_environment_t envCallback;

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
    };
}
#endif