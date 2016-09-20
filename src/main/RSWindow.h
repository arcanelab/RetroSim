//
//  RSWindow.h
//  RetroSim
//
//  Created by Zoltán Majoros on 06/Nov/15.
//  Copyright © 2015 Zoltán Majoros. All rights reserved.
//
// Generic SDL window class
//

#pragma once

#include "RSTypes.h"

class SDL_Window;
class SDL_Renderer;
class SDL_Color;

struct RSWindow
{
    SDL_Window *window = nullptr;
    SDL_Renderer *renderer = nullptr;
    SDL_Color *bgColor = nullptr;
    uint32 windowID = 0;
    
    int width = 0, height = 0; // logical coordinates
    int pixelWidth = 0, pixelHeight = 0; // physical coordinates, in HiDPI modes they're different from width/height
    double highDPIScale = 1.0; // the ratio between the physical and logical coordinates
    
    virtual void create(const char *title, int width, int height, int posX, int posY, bool isResizable = false);
    void clear();
    void show();
    virtual void close();
    virtual void textInput(char characters[32]);
    virtual void keyInput(int scancode);
    virtual void resized(uint32 newWidth, uint32 newHeight);
    RSWindow();
    virtual ~RSWindow();
};
