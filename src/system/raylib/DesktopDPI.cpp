// RetroSim - Copyright 2011-2023 Zoltán Majoros. All rights reserved.
// https://github.com/arcanelab

#if defined(_WIN32)
#include <windows.h>
#endif

#if defined(__APPLE__)
#include <objc/objc.h>
#include <objc/message.h>
#endif

namespace RetroSim
{
    float GetDesktopScalingFactor()
    {
#if defined(_WIN32)
        HWND hwnd = GetConsoleWindow(); // Or get your actual game window handle if not using the console
        int dpi = GetDpiForWindow(hwnd);
        return (float)dpi / 96.0f;
#else
        // On Mac, it's always 1.0f.
        // TODO: implement Linux, etc.
        return 1.0f; // Default scale for other platforms
#endif
    }
}
