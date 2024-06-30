// RetroSim - Copyright 2011-2023 Zoltán Majoros. All rights reserved.
// https://github.com/arcanelab

#if defined(_WIN32)
#include "windows.h"
#include <cmath>
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
        HWND hwnd = GetForegroundWindow();
        int dpi = GetDpiForWindow(hwnd);
        float scalingFactor = static_cast<float>(dpi) / 96.0f;
        // if it's 0, return 1.
        return std::fpclassify(scalingFactor) == FP_ZERO ? 1.0f : scalingFactor;
#else
        // On Mac it's always 1.0f.
        // TODO: implement Linux, etc.
        return 1.0f; // Default scale for other platforms
#endif
    }
}
