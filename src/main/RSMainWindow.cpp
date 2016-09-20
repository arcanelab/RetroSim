//
//  RSMainWindow.cpp
//  RetroSim
//
//  Created by Zoltán Majoros on 20/May/2015.
//  Copyright © 2016 Zoltán Majoros. All rights reserved.
//

#include "RSMainWindow.h"
#include "RSCore.h"
#include "RSFonts.h"
#include <SDL2/SDL_TTF.h>

void RSMainWindow::create(const char *title, int width, int height, int posX, int posY, bool isResizable)
{
    RSWindow::create(title, width, height, posX, posY);
    
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width, height);
    if(texture == nullptr)
    {
        puts("Error while creating SDL surface.\n");
        throw;
    }
    
    font = RSFont::getMonoFont(fontSize * (int)highDPIScale, false);
}

void RSMainWindow::render(RSCore *core)
{
    SDL_UpdateTexture(texture, NULL, core->frameBuffer, core->width << 2); // copy VM framebuffer into SDL texture
    SDL_RenderCopy(renderer, texture, NULL, NULL);
}

void RSMainWindow::displayFrameDuration(double dt)
{
    char text[40];
    //        if(!(frameCounter % 20))
    sprintf(text, "frame time = %.1f microsec", dt); // convert double to c-string
    SDL_Color color = {255,255,255, 255};
    SDL_Surface *textSurface = TTF_RenderUTF8_Blended(font, text, color);
    SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    if(textSurface == NULL)
    {
        puts("Error rendering text\n");
        throw;
    }
    int w, h;
    SDL_QueryTexture(textTexture, NULL, NULL, &w, &h);
    SDL_Rect rect = {5,0,w,h};
    SDL_RenderCopy(renderer, textTexture, NULL, &rect);
    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
}

void RSMainWindow::close()
{
    RSWindow::close();
    
    if(texture)
        SDL_DestroyTexture(texture);
    texture = nullptr;
}

