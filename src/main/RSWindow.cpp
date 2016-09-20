//
//  RSWindow.cpp
//  RetroSim
//
//  Created by Zoltán Majoros on 20/May/2015.
//  Copyright © 2016 Zoltán Majoros. All rights reserved.
//

#include "RSWindow.h"
#include <SDL2/SDL.h>

void RSWindow::create(const char *title, int width, int height, int posX, int posY, bool isResizable)
{
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    
    uint32 isResizableValue = isResizable ? SDL_WINDOW_RESIZABLE : 0;
    window = SDL_CreateWindow(title, posX, posY, width, height, SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_OPENGL | isResizableValue);
    if(window == nullptr)
    {
        puts("Error while creatign SDL window.");
        SDL_Quit();
        throw;
    }

    bgColor->r = 34;
    bgColor->g = 34;
    bgColor->b = 34;
    bgColor->a = 255;
    
    SDL_GL_SetSwapInterval(1);
    
    windowID = SDL_GetWindowID(window);
    
    this->width = width;
    this->height = height;
    
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if(renderer == nullptr)
    {
        puts("Error while creatign SDL renderer.");
        SDL_Quit();
        throw;
    }
    
    SDL_GL_GetDrawableSize(window, &pixelWidth, &pixelHeight);
    highDPIScale = (double)pixelWidth / (double)width;
}

void RSWindow::clear()
{
    SDL_SetRenderDrawColor(renderer, bgColor->r, bgColor->g, bgColor->b, bgColor->a);
    SDL_RenderClear(renderer);
}

void RSWindow::show()
{
    SDL_RenderPresent(renderer); // waits until vsync
}

void RSWindow::close()
{
    if(window)
        SDL_DestroyWindow(window);
    
    window = nullptr;
}

void RSWindow::textInput(char characters[32])
{
}

void RSWindow::keyInput(int scancode)
{
#if 0
    switch(scancode)
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
            break;
        default:
            break;
    }
#endif
}

void RSWindow::resized(uint32 newWidth, uint32 newHeight)
{
    width = (int)newWidth;
    height = (int)newHeight;
    
    SDL_GL_GetDrawableSize(window, &pixelWidth, &pixelHeight);
}

RSWindow::RSWindow()
{
    bgColor = new SDL_Color;
}

RSWindow::~RSWindow()
{
    close();
    delete bgColor;
}

