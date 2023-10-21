#include "Logger.h" // Replace with the correct path to your header file

#include <cstdarg>

namespace RetroSim::Logger
{
#ifdef LIBRETRO
    retro_log_printf_t LogPrintf = nullptr;

    void SetLibRetroCallback(retro_log_printf_t libRetroPrintf)
    {
        LogPrintf = libRetroPrintf;
    }
#else
    log_printf_t LogPrintf = log_printf;

    void log_printf(enum retro_log_level level, const char *fmt, ...)
    {
        va_list args;
        va_start(args, fmt);
        vprintf(fmt, args);
        va_end(args);
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
}
