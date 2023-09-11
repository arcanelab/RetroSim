// RetroSim - Copyright 2011-2023 Zoltán Majoros. All rights reserved.
// https://github.com/arcanelab

#include "GPU.h"
#include "Core.h"
#include "MMU.h"
#ifdef __APPLE__ // or shall we use __clang__?
#include <stddef.h>
#endif

namespace RetroSim::GPU
{
    uint32_t *outputTexture = new uint32_t[textureWidth * textureHeight]; // ARGB8888;

    // clipping rectangle
    uint16_t clipX0 = 0;
    uint16_t clipY0 = 0;
    uint16_t clipX1 = 0;
    uint16_t clipY1 = 0;

    // for print()
    uint8_t fontWidth = 8;
    uint8_t fontHeight = 16;
    uint32_t fontOffset = 0; // defines how many characters to skip at the start of CHARSET

    void Initialize()
    {
        memset(outputTexture, 0, textureSize);
        MMU::WriteMem<uint8_t>(MMU::TILE_WIDTH, 8);
        MMU::WriteMem<uint8_t>(MMU::TILE_HEIGHT, 16);
        MMU::WriteMem<uint8_t>(MMU::MAP_WIDTH, 30);
        MMU::WriteMem<uint8_t>(MMU::MAP_HEIGHT, 16);
        MMU::WriteMem<uint8_t>(MMU::PALETTE_BANK, 0);
        MMU::WriteMem<uint8_t>(MMU::MAP_BANK, 0);
        MMU::WriteMem<uint8_t>(MMU::TILE_BANK, 0);
        MMU::WriteMem<uint8_t>(MMU::SPRITE_BANK, 0);
    }

    void SetFont(int width, int height, int offset = 0)
    {
        fontWidth = width;
        fontHeight = height;
        fontOffset = offset;
    }

    void PrintText(const char *text, int x, int y, int color, int scale, int16_t transparentColorIndex)
    {
        int characterCount = 0;
        while (char c = *text++)
        {
            for (int k = 0; k < fontHeight; k++)
                for (int j = 0; j < fontWidth; j++)
                {
                    uint8_t colorIndex = MMU::ReadMem<uint8_t>(MMU::CHARSET + fontOffset + c * fontWidth * fontHeight + k * fontWidth + j);

                    if (transparentColorIndex != -1 && colorIndex == transparentColorIndex)
                        continue;

                    DrawPixel(x + j, y + k, color);
                }

            x += fontWidth * scale;
        }
    }

    void ClearScreen()
    {
        for (int y = 0; y < textureHeight; y++)
            for (int x = 0; x < textureWidth; x++)
                DrawPixel(x, y, 0);
    }

    void ClearScreenIgnoreClipping()
    {
        memset(outputTexture, 0, textureSize);
    }

    void DrawLine(int x0, int y0, int x1, int y1, int color)
    {
        int dx = abs(x1 - x0);
        int dy = abs(y1 - y0);
        int sx = x0 < x1 ? 1 : -1;
        int sy = y0 < y1 ? 1 : -1;
        int err = dx - dy;

        while (true)
        {
            DrawPixel(x0, y0, color);

            if (x0 == x1 && y0 == y1)
                break;

            int e2 = 2 * err;
            if (e2 > -dy)
            {
                err -= dy;
                x0 += sx;
            }

            if (e2 < dx)
            {
                err += dx;
                y0 += sy;
            }
        }

        DrawPixel(x1, y1, color);
    }

    void DrawCircle(int x, int y, int radius, int color, bool filled)
    {
        // Compute the coordinates of the center of the circle
        int cx = x;
        int cy = y;

        // Compute the squared radius of the circle
        int r2 = radius * radius;

        // Loop over the pixels in the bounding box of the circle
        for (int py = y - radius; py <= y + radius; py++)
        {
            for (int px = x - radius; px <= x + radius; px++)
            {
                // Compute the distance between the pixel and the center of the circle
                int dx = px - cx;
                int dy = py - cy;
                int dx2 = dx * dx;
                int dy2 = dy * dy;
                int d2 = dx2 + dy2;

                // Check if the pixel is inside the circle
                if (d2 <= r2)
                {
                    // Draw the pixel
                    if (filled || d2 >= r2 - 2 * radius)
                    {
                        DrawPixel(px, py, color);
                    }
                    else if ((dx2 + (dy - 1) * (dy - 1)) > r2 || (dx2 + (dy + 1) * (dy + 1)) > r2 || ((dx - 1) * (dx - 1) + dy2) > r2 || ((dx + 1) * (dx + 1) + dy2) > r2)
                    {
                        DrawPixel(px, py, color);
                    }
                }
            }
        }
    }

    void DrawRect(int x, int y, int width, int height, int color, bool filled)
    {
        if (filled)
        {
            for (int i = 0; i < height; i++)
                DrawLine(x, y + i, x + width, y + i, color);
        }
        else
        {
            DrawLine(x, y, x + width, y, color);
            DrawLine(x + width, y, x + width, y + height, color);
            DrawLine(x + width, y + height, x, y + height, color);
            DrawLine(x, y + height, x, y, color);
        }
    }

    void DrawTriangle(int x0, int y0, int x1, int y1, int x2, int y2, int color, bool filled)
    {
        DrawLine(x0, y0, x1, y1, color);
        DrawLine(x1, y1, x2, y2, color);
        DrawLine(x2, y2, x0, y0, color);
    }

    void DrawTexturedTriangle(int x0, int y0, int x1, int y1, int x2, int y2, int u0, int v0, int u1, int v1, int u2, int v2)
    {
    }

    void DrawPixel(int x, int y, uint8_t colorIndex)
    {
        if (x < 0 || x >= textureWidth || y < 0 || y >= textureHeight)
            return;

        if (x < clipX0 || x > clipX1 || y < clipY0 || y > clipY1)
            return;

        outputTexture[x + y * textureWidth] = GetPaletteColor(colorIndex);
    }

    void SetClipping(int x0, int y0, int x1, int y1)
    {
        clipX0 = x0;
        clipY0 = y0;
        clipX1 = x1;
        clipY1 = y1;
    }

    void DisableClipping()
    {
        clipX0 = 0;
        clipY0 = 0;
        clipX1 = textureWidth - 1;
        clipY1 = textureHeight - 1;
    }

    void DrawMap(int screenX, int screenY, int mapX, int mapY, int width, int height, int16_t transparentColorIndex)
    {
        uint8_t tileWidth = MMU::ReadMem<uint8_t>(MMU::TILE_WIDTH);
        uint8_t tileHeight = MMU::ReadMem<uint8_t>(MMU::TILE_HEIGHT);
        uint8_t mapWidth = MMU::ReadMem<uint8_t>(MMU::MAP_WIDTH);
        uint8_t mapHeight = MMU::ReadMem<uint8_t>(MMU::MAP_HEIGHT);

        for (int tileY = mapY; tileY < mapY + height; tileY++)
        {
            for (int tileX = mapX; tileX < mapX + width; tileX++)
            {
                int tileIndex = MMU::ReadMem<uint8_t>(MMU::MAP_U8 + tileX + tileY * mapWidth);
                int tileBitmapAddress = MMU::TILES_U8 + tileIndex * tileWidth * tileHeight;

                for (int tileMemY = 0; tileMemY < tileHeight; tileMemY++)
                {
                    for (int tileMemX = 0; tileMemX < tileWidth; tileMemX++)
                    {
                        int colorIndex = MMU::ReadMem<uint8_t>(tileBitmapAddress + tileMemX + tileMemY * tileWidth);
                        if (transparentColorIndex != -1 && colorIndex == transparentColorIndex)
                            continue;

                        int pixelCoordX = screenX + (tileX - mapX) * tileWidth + tileMemX;
                        int pixelCoordY = screenY + (tileY - mapY) * tileHeight + tileMemY;

                        DrawPixel(pixelCoordX, pixelCoordY, colorIndex);
                    }
                }
            }
        }
    }

    void DrawSprite(int screenPosX, int screenPosY, int spritePosX, int spritePosY, int width, int height, int16_t transparentColorIndex)
    {
        const int atlasWidth = 128;
        const uint16_t spriteDataOffset = spritePosX + spritePosY * atlasWidth;

        for (int spriteY = 0; spriteY < height; spriteY++)
        {
            for (int spriteX = 0; spriteX < width; spriteX++)
            {
                int colorIndex = MMU::ReadMem<uint8_t>(MMU::SPRITE_ATLAS_U8 + spriteDataOffset + spriteX + spriteY * atlasWidth);
                if (transparentColorIndex != -1 && colorIndex == transparentColorIndex)
                    continue;
                DrawPixel(screenPosX + spriteX, screenPosY + spriteY, GetPaletteColor(colorIndex));
            }
        }
    }

    void DrawBitmap(int screenPosX, int screenPosY, int bitmapPosX, int bitmapPosY, int width, int height, int pitch, int16_t transparentColorIndex)
    {
        for (int y = 0; y < height; y++)
        {
            int bitmapAddress = MMU::BITMAP_U8 + bitmapPosX + (bitmapPosY + y) * pitch;
            for (int x = 0; x < width; x++)
            {
                int colorIndex = MMU::ReadMem<uint8_t>(bitmapAddress + x);
                if (transparentColorIndex != -1 && colorIndex == transparentColorIndex)
                    continue;
                DrawPixel(screenPosX + x, screenPosY + y, colorIndex);
            }
        }
    }

    void SetPaletteBank(int bank)
    {
    }

    void SetTileBank(int bank)
    {
    }

    void SetSpriteBank(int bank)
    {
    }

    void SetPaletteColor(int index, int r, int g, int b)
    {
        MMU::WriteMem<uint32_t>(MMU::PALETTE_U32 + index * 4, r << 16 | g << 8 | b);
    }

    uint32_t GetPaletteColor(uint8_t colorIndex)
    {
        return MMU::ReadMem<uint32_t>(MMU::PALETTE_U32 + colorIndex * 4);
    }
}
