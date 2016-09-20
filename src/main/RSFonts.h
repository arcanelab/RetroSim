//
//  RSFonts.h
//  RetroSim
//
//  Created by Zoltán Majoros on 09/Nov/15.
//  Copyright © 2015 Zoltán Majoros. All rights reserved.
//

#pragma once
class _TTF_Font;

struct RSFont
{
    static _TTF_Font *monoFont;
    static _TTF_Font *monoFontBold;
    
    static _TTF_Font *getMonoFont(int size, bool isBold);
};
// http://www.fontsquirrel.com/fonts/list/classification/monospaced
// https://fontlibrary.org/en/search?category=monospaced

//#define FONTNAME_NORMAL "fonts/DOS VGA.ttf"
//#define FONTNAME_BOLD "fonts/DOS VGA.ttf"

//#define FONTNAME_NORMAL "fonts/Petme1282Y.ttf"
//#define FONTNAME_BOLD "fonts/Petme1282Y.ttf"

//#define FONTNAME_NORMAL "fonts/Lekton.ttf"
//#define FONTNAME_BOLD "fonts/Lekton.ttf"

//#define FONTNAME_NORMAL "fonts/Anonymous Pro.ttf"
//#define FONTNAME_BOLD "fonts/Anonymous Pro B.ttf"

//#define FONTNAME_BOLD "fonts/VeraMono-Bold.ttf"
//#define FONTNAME_NORMAL "fonts/VeraMono.ttf"

