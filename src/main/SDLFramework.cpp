//
//  SDLFramework.cpp
//  RetroSim
//
//  Created by Zoltán Majoros on 20/May/2015.
//  Copyright © 2016 Zoltán Majoros. All rights reserved.
//

#include "SDLFramework.h"
#include "RSFile.h"
#include "RSCore.h"
#include "A65000CPU.h"
#include "RSTime.h"
#include "RSmainWindow.h"
#include "RSdebugWindow.h"
#include "RSMonitorInterpreter.h"

#include <SDL2/SDL_image.h>

int SDLFramework::setupSDLandCore()
{
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
    {
        puts("Error initializing SDL2.");
        return -1;
    }
    
    if(IMG_Init( IMG_INIT_PNG)!= IMG_INIT_PNG)
    {
        puts("Error while initializing SDL2_image.");
        SDL_Quit();
        return -1;
    }
    
    mainWindow = new RSMainWindow;
    debugWindow = new RSDebugWindow;
    
    mainWindow->create("Retrosim", 680, 512, 190, 190);
    debugWindow->create("Debugger", 850, 600, 1000, 140);
    
    core = new RSCore(mainWindow->width, mainWindow->height);
    debugWindow->monitorInterpreter->setCore(core);
    
    system("pwd");
    
    return 0;
}

void SDLFramework::cleanupSDL()
{
    mainWindow->close();
    debugWindow->close();
    delete mainWindow;
    delete debugWindow;
    
    IMG_Quit();
    SDL_Quit();
}

// -----------------------------------------------------------

int SDLFramework::main()
{
    if(setupSDLandCore())
        return -1;
    
    loadAndExecuteStartupScript();
/*
    for(int i=0; i<core->rgbSurface.width * core->rgbSurface.height; i++)
        core->mmu.write<uint32>(core->rgbSurface.framebufferPtr+i*4, 0x7fcfff);
*/
//    if(runCPUThread()) return -1;    
    SDL_Event event;
    while(running)
    {
        while(SDL_PollEvent(&event))
        {
            switch (event.type)
            {
                case SDL_QUIT:
                    running = false;
                    break;
                    /*                    case SDL_WINDOWEVENT:
                     if(event.window.event == SDL_WINDOWEVENT_RESIZED)
                     {
                     SDL_GL_GetDrawableSize(mainWindow->window, &pixelWidth, &pixelHeight);
                     }
                     break;*/
                case SDL_MOUSEBUTTONUP:
                    switch(event.button.button)
                {
                    case SDL_BUTTON_RIGHT:
                        break;
                    case SDL_BUTTON_LEFT:
                        break;
                    default:
                        break;
                }
                    break;
                    
                case SDL_WINDOWEVENT:
                    switch(event.window.event)
                {
                    case SDL_WINDOWEVENT_FOCUS_GAINED:
                        //                                printf("focus gained: %d\n", event.window.windowID);
                        windowWithFocus = SDL_GetWindowFromID(event.window.windowID);
                        
                        if(event.window.windowID == debugWindow->windowID) // TODO: move these INTO debugWindow->cpp, jeeez
                        {                                                 // just pass on the focus notification
                            debugWindow->cursorEnabled = true;
                            debugWindow->cursorVisible = true;
                        }
                        
                        break;
                    case SDL_WINDOWEVENT_FOCUS_LOST:
                        //                                printf("focus lost: %d\n", event.window.windowID);
                        
                        windowWithFocus = nullptr;
                        
                        if(event.window.windowID == debugWindow->windowID)
                        {
                            debugWindow->cursorEnabled = false;
                            debugWindow->cursorVisible = false;
                        }
                        break;
                    case SDL_WINDOWEVENT_RESIZED:
                        if(event.window.windowID == debugWindow->windowID)
                        {
                            debugWindow->resized(event.window.data1, event.window.data2);
                        }
                        break;
                }
                    break;
                    
                case SDL_KEYDOWN:
                {
                    RSWindow *targetWindow = nullptr;
                    
                    if(mainWindow->windowID == event.window.windowID)
                        targetWindow = mainWindow;
                    else if (debugWindow->windowID == event.window.windowID)
                        targetWindow = debugWindow;
                    else
                        break;
                    
                    switch(event.key.keysym.scancode)
                    {
                        case SDL_SCANCODE_BACKSPACE:
                        case SDL_SCANCODE_RETURN:
                        case SDL_SCANCODE_RETURN2:
                        case SDL_SCANCODE_LEFT:
                        case SDL_SCANCODE_RIGHT:
                        case SDL_SCANCODE_DOWN:
                        case SDL_SCANCODE_UP:
                        case SDL_SCANCODE_TAB:
                        case SDL_SCANCODE_ESCAPE:
                        case SDL_SCANCODE_PAGEUP:
                        case SDL_SCANCODE_PAGEDOWN:
                        case SDL_SCANCODE_HOME:
                        case SDL_SCANCODE_INSERT:
                        case SDL_SCANCODE_F1:
                        case SDL_SCANCODE_F2:
                        case SDL_SCANCODE_F3:
                        case SDL_SCANCODE_F4:
                        case SDL_SCANCODE_F5:
                        case SDL_SCANCODE_F6:
                        case SDL_SCANCODE_F7:
                        case SDL_SCANCODE_F8:
                        case SDL_SCANCODE_F9:
                        case SDL_SCANCODE_F10:
                        case SDL_SCANCODE_F11:
                        case SDL_SCANCODE_F12:
                        case SDL_SCANCODE_LALT:
                        case SDL_SCANCODE_RALT:
                        case SDL_SCANCODE_LCTRL:
                        case SDL_SCANCODE_RCTRL:
                        case SDL_SCANCODE_LSHIFT:
                        case SDL_SCANCODE_RSHIFT:
                            targetWindow->keyInput(event.key.keysym.scancode);
                            break;
                        default:
                            break;
                    }
                    break;
                }
                    
                case SDL_TEXTINPUT:
                {
                    RSWindow *targetWindow = nullptr;
                    
                    if(mainWindow->windowID == event.window.windowID)
                        targetWindow = mainWindow;
                    else if (debugWindow->windowID == event.window.windowID)
                        targetWindow = debugWindow;
                    else
                        throw;
                    
                    targetWindow->textInput(event.text.text);
                }
                default:
                    break;
            }
        }
        
        t1 = RSTime::getElapsedTimeInMicroseconds();
        double dt = t1 - t0;
        t0 = RSTime::getElapsedTimeInMicroseconds();
        
        mainWindow->clear();
        mainWindow->render(core);
        mainWindow->displayFrameDuration(dt);
        mainWindow->show();
        
        debugWindow->clear();
        debugWindow->render();
        debugWindow->show();
        
        core->updateFrame(17);
        frameCounter++;
    }
    
    cleanupSDL();
    
    return 0;
}

void SDLFramework::loadAndExecuteStartupScript()
{
    RSTextFileIn file("startup.rc");
    if(!file.success)
    {
        puts(file.errorMessage.c_str());
        return;
    }
    for(string line: file.lines)
        debugWindow->processInputFromFramework(line);
}


/*
 #if 0 // alternative update method, faster, but not as simple as SDL_UpdateTexture()
 SDL_LockTexture(texture, NULL, &pixels, &pitch); // TODO: handle non-width wide pitch
 void *pixels;
 int pitch;
 
 for(int i=0; i<pitch * core->height / 2; i++)
 ((uint64 *)pixels)[i] = ((uint64 *)core->frameBuffer)[i];
 
 SDL_UnlockTexture(texture);
 #endif
 */
