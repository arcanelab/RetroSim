// RetroSim - Copyright 2011-2023 Zoltán Majoros. All rights reserved.
// https://github.com/arcanelab

#pragma once
#include <cstdint>

namespace RetroSim::GPU
{
    const uint16_t windowWidth = 480;
    const uint16_t windowHeight = 270;
    const uint_fast16_t textureWidth = 464;
    const uint_fast16_t textureHeight = 256;
    const uint_fast32_t pixelCount = textureWidth * textureHeight;
    const uint_fast32_t textureSizeInBytes = pixelCount * 4;

    extern uint32_t *outputTexture; // ARGB8888;

    void Initialize();

    enum APICalls
    {
        SetFontID,
        SetPaletteColorID,
        RenderTextID,
        RenderOpaqueTextID,
        ClearScreenID,
        ClearScreenIgnoreClippingID,
        DrawPixelID,
        DrawLineID,
        DrawRectID,
        DrawCircleID,
        DrawTriangleID,
        DrawTexturedTriangleID,
        DrawMapID,
        DrawSpriteID,
        DrawBitmapID,
        SetClippingID,
        DisableClippingID,
    };

    // API
    void SetFont(int width, int height, int offset);
    void SetPaletteColor(int index, int r, int g, int b);
    void RenderText(const char *text, int x, int y, uint8_t colorIndex);
    void RenderOpaqueText(const char *text, int x, int y, uint8_t colorIndex, uint8_t backgroundColorIndex);
    void ClearScreen(uint8_t colorIndex = 0);
    void ClearScreenIgnoreClipping(uint8_t colorIndex = 0);
    void DrawPixel(int x, int y, uint8_t colorIndex);
    void DrawLine(int x0, int y0, int x1, int y1, uint8_t colorIndex);
    void DrawRect(int x, int y, int width, int height, uint8_t colorIndex, bool filled);
    void DrawCircle(int x, int y, int radius, uint8_t colorIndex, bool filled);
    void DrawTriangle(int x0, int y0, int x1, int y1, int x2, int y2, uint8_t colorIndex, bool filled);
    void DrawTexturedTriangle(int x0, int y0, int x1, int y1, int x2, int y2, int u0, int v0, int u1, int v1, int u2, int v2);
    void DrawMap(int screenX, int screenY, int mapX, int mapY, int width, int height, int16_t transparentColorIndex);
    void DrawSprite(int x, int y, int spritex, int spritey, int width, int height, int16_t transparentColorIndex);
    void DrawBitmap(int screenPosX, int screenPosY, int bitmapPosX, int bitmapPosY, int width, int height, int pitch, int16_t transparentColorIndex = -1);
    void SetClipping(int x0, int y0, int x1, int y1);
    void DisableClipping();

    // Helper functions
    uint32_t GetPaletteColor(uint8_t colorIndex);
}
