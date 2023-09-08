// RetroSim - Copyright 2011-2023 Zoltán Majoros. All rights reserved.
// https://github.com/arcanelab

#pragma once
#include <stdio.h>
#include <iostream>
#ifdef LIBRETRO
#include "libretro/libretro-common/include/libretro.h"
#endif
#include "CoreConfig.h"

namespace RetroSim
{
    class Logger
    {
    public:
#ifdef LIBRETRO
        retro_log_printf_t Printf = nullptr;

        void SetLibRetroCallback(retro_log_printf_t libRetroPrintf)
        {
            this->Printf = libRetroPrintf;
        }
#endif

        void Log(const char *message)
        {
            printf("%s\n", message);
        }

        void Log(std::string message)
        {
            std::cout << message << std::endl;
        }
    };
} // namespace RetroSim