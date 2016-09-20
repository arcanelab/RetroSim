//
//  RSFonts.cpp
//  RetroSim
//
//  Created by Zoltán Majoros on 20/May/2015.
//  Copyright © 2016 Zoltán Majoros. All rights reserved.
//

#include "RSFonts.h"
#include <SDL2/SDL_TTF.h>

#define FONTNAME_NORMAL "fonts/Menlo-Regular.ttf" // default
#define FONTNAME_BOLD "fonts/Menlo-Bold.ttf"

TTF_Font *RSFont::monoFont;
TTF_Font *RSFont::monoFontBold;

TTF_Font* RSFont::getMonoFont(int size, bool isBold)
{
    if(TTF_WasInit() == 0)
    {
        if(TTF_Init() == -1)
        {
            puts("Error while initializing SDL2_TTF.");
            SDL_Quit();
            throw;
        }
    }
    
    if(isBold)
    {
        if(monoFontBold == nullptr)
        {
            const char *fontNameBold = FONTNAME_BOLD;
            
            monoFontBold = TTF_OpenFont(fontNameBold, size);
            if(monoFontBold == nullptr)
            {
                printf("Error opening font: %s", fontNameBold);
                SDL_Quit();
                throw;
            }
        }
        
        return monoFontBold;
    }
    else if(monoFont == nullptr)
    {
        const char *fontNameBold = FONTNAME_BOLD;
        const char *fontNameNormal = FONTNAME_NORMAL;
        
        printf("Filename: %s\n", isBold?fontNameBold:fontNameNormal);
        
        monoFont = TTF_OpenFont(isBold?fontNameBold:fontNameNormal, size);
        if(monoFont == nullptr)
        {
            printf("Error opening font: %s", isBold?fontNameBold:fontNameNormal);
            SDL_Quit();
            throw;
        }
        
        return monoFont;
    }
    
    return monoFont;
}
