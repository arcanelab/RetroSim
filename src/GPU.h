// RetroSim - Copyright 2011-2023 Zoltán Majoros. All rights reserved.
// https://github.com/arcanelab

#pragma once
#include <cstdint>

namespace RetroSim::GPU
{
    const uint_fast16_t textureWidth = 480;
    const uint_fast16_t textureHeight = 256;
    const uint_fast32_t pixelCount = textureWidth * textureHeight;
    const uint_fast32_t textureSize = pixelCount * 4;

    extern uint32_t *outputTexture; // ARGB8888;

    extern uint32_t frameNumber;

    void Print(const char *text, int x, int y, int color, int transparent, int scale);
    void Cls();
    void Line(int x0, int y0, int x1, int y1, int color);
    void Circle(int x, int y, int radius, int color, bool filled);
    void Rect(int x, int y, int width, int height, int color, bool filled);
    void Tri(int x0, int y0, int x1, int y1, int x2, int y2, int color, bool filled);
    void Tex(int x0, int y0, int x1, int y1, int x2, int y2, int u0, int v0, int u1, int v1, int u2, int v2);
    void Pixel(int x, int y, uint8_t colorIndex);
    void Clip(int x0, int y0, int x1, int y1);
    void NoClip();
    void Map(int x, int y, int mapx, int mapy, int width, int height);
    void Sprite(int x, int y, int spritex, int spritey, int width, int height);
    void Palette(int bank);
    void Tiles(int bank);
    void Sprite(int bank);
    void PalColor(int index, int r, int g, int b);
    void Bitmap(int x, int y, int bitmapx, int bitmapy, int width, int height);
}
