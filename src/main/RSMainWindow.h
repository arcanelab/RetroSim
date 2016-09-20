//
//  RSMainWindow.h
//  RetroSim
//
//  Created by Zoltán Majoros on 07/Nov/15.
//  Copyright © 2015 Zoltán Majoros. All rights reserved.
//

#pragma once

#include "RSWindow.h"

class RSCore;
class SDL_Texture;
class _TTF_Font;

struct RSMainWindow : public RSWindow
{
    SDL_Texture *texture = nullptr;
    _TTF_Font *font = nullptr;
    int fontSize = 16;
    
    virtual void create(const char *title, int width, int height, int posX, int posY, bool isResizable = false) override;
    void render(RSCore *core);
    void displayFrameDuration(double dt);
    virtual void close() override;
};
