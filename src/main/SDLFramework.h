//
//  SDLFramework.h
//
//  Created by Zoltán Majoros on 23/Mar/15.
//  Copyright (c) 2015 Zoltán Majoros. All rights reserved.
//
// SDLFramework: here lies the main structure of the application.
// SDL initialization, main loop, event handling, timers, etc is implemented here.
// In other words, this is the layer between the Virtual Machine and the
// operating system that handles windows, user input, time handling, threads, etc.
// We instantiate and update the VM core from here as well.
//

#pragma once
#include "RSTypes.h"

class RSMainWindow;
class RSDebugWindow;
class RSCore;
class SDL_Window;

struct SDLFramework
{
    RSCore *core = nullptr;
    
    RSMainWindow *mainWindow = nullptr;
    RSDebugWindow *debugWindow = nullptr;
    SDL_Window *windowWithFocus = nullptr;
    
    double t0 = 0.0, t1 = 0.0, dt = 0.0;
    uint64 frameCounter = 0;

    bool running = true;
    
    int main();

    int setupSDLandCore();
    void cleanupSDL();
    
    void loadAndExecuteStartupScript();
};

