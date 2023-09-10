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
    uint16_t clipX0 = 0;
    uint16_t clipY0 = 0;
    uint16_t clipX1 = 0;
    uint16_t clipY1 = 0;

    uint32_t frameNumber = 0;

    // void Render(const uint8_t tileWidth, const uint8_t tileHeight)
    // {
    //     const uint8_t numTilesX = textureWidth / tileWidth;
    //     const uint8_t numTilesY = textureHeight / tileHeight;

    //     uint8_t tileIndex;
    //     uint8_t colorIndex;
    //     uint32_t color;

    //     size_t pixelCoordX;
    //     size_t pixelCoordY;

    //     uint16_t tileBitmapAddress;

    //     for (size_t tileMapY = 0; tileMapY < numTilesY; tileMapY++)
    //     {
    //         for (size_t tileMapX = 0; tileMapX < numTilesX; tileMapX++)
    //         {
    //             tileIndex = MMU::ReadMem<uint8_t>(uint32_t(Core::MAP_MEMORY_U8 + tileMapX + tileMapY * (size_t)numTilesX));
    //             tileBitmapAddress = Core::TILE_MEMORY_U8 + tileIndex * tileWidth * tileHeight;

    //             for (size_t tileMemY = 0; tileMemY < tileHeight; tileMemY++)
    //             {
    //                 for (size_t tileMemX = 0; tileMemX < tileWidth; tileMemX++)
    //                 {
    //                     colorIndex = MMU::ReadMem<uint8_t>(uint32_t(tileBitmapAddress + tileMemX + tileMemY * tileWidth));
    //                     color = MMU::ReadMem<uint32_t>(Core::PALETTE_MEMORY_U32 + colorIndex * 4);

    //                     pixelCoordX = tileMapX * tileWidth + tileMemX;
    //                     pixelCoordY = tileMapY * tileHeight + tileMemY;

    //                     outputTexture[pixelCoordX + pixelCoordY * textureWidth] = color;
    //                 }
    //             }
    //         }
    //     }
    // }

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

    uint8_t fontWidth = 8;
    uint8_t fontHeight = 16;
    uint32_t fontOffset = 0; // defines how many characters to skip at the start of CHARSET

    void SetFont(int width, int height, int offset = 0)
    {
        fontWidth = width;
        fontHeight = height;
        fontOffset = offset;
    }

    void Print(const char *text, int x, int y, int color, int transparent = 0, int scale = 1)
    {
        int characterCount = 0;
        while (char c = *text++)
        {
            for (int k = 0; k < fontHeight; k++)
                for (int j = 0; j < fontWidth; j++)
                {
                    uint8_t colorIndex = MMU::ReadMem<uint8_t>(MMU::CHARSET + fontOffset + c * fontWidth * fontHeight + k * fontWidth + j);

                    if (colorIndex == transparent)
                        continue;

                    Pixel(x + j, y + k, color);
                }

            x += fontWidth * scale;
        }
    }

    void Cls()
    {
        // clear screen using Pixel()
        for (int y = 0; y < textureHeight; y++)
            for (int x = 0; x < textureWidth; x++)
                Pixel(x, y, 0);
    }

    void ClsNoClip()
    {
        memset(outputTexture, 0, textureSize);
    }

    void Line(int x0, int y0, int x1, int y1, int color)
    {
        int dx = abs(x1 - x0);
        int dy = abs(y1 - y0);
        int sx = x0 < x1 ? 1 : -1;
        int sy = y0 < y1 ? 1 : -1;
        int err = dx - dy;

        while (true)
        {
            Pixel(x0, y0, color);

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

        Pixel(x1, y1, color);
    }

    void Circle(int x, int y, int radius, int color, bool filled)
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
                        Pixel(px, py, color);
                    }
                    else if ((dx2 + (dy - 1) * (dy - 1)) > r2 || (dx2 + (dy + 1) * (dy + 1)) > r2 || ((dx - 1) * (dx - 1) + dy2) > r2 || ((dx + 1) * (dx + 1) + dy2) > r2)
                    {
                        Pixel(px, py, color);
                    }
                }
            }
        }
    }

    void Rect(int x, int y, int width, int height, int color, bool filled)
    {
        if (filled)
        {
            for (int y = 0; y < height; y++)
                Line(x, y, x + width, y, color);
        }
        else
        {
            Line(x, y, x + width, y, color);
            Line(x + width, y, x + width, y + height, color);
            Line(x + width, y + height, x, y + height, color);
            Line(x, y + height, x, y, color);
        }
    }

    void Tri(int x0, int y0, int x1, int y1, int x2, int y2, int color, bool filled)
    {
        Line(x0, y0, x1, y1, color);
        Line(x1, y1, x2, y2, color);
        Line(x2, y2, x0, y0, color);
    }

    void Tex(int x0, int y0, int x1, int y1, int x2, int y2, int u0, int v0, int u1, int v1, int u2, int v2)
    {
    }

    void Pixel(int x, int y, uint8_t colorIndex)
    {
        if (x < 0 || x >= textureWidth || y < 0 || y >= textureHeight)
            return;

        if (x < clipX0 || x > clipX1 || y < clipY0 || y > clipY1)
            return;

        uint32_t color = MMU::ReadMem<uint32_t>(MMU::PALETTE_U32 + colorIndex * 4);
        outputTexture[x + y * textureWidth] = color;
    }

    void Clip(int x0, int y0, int x1, int y1)
    {
        clipX0 = x0;
        clipY0 = y0;
        clipX1 = x1;
        clipY1 = y1;
    }

    void NoClip()
    {
        clipX0 = 0;
        clipY0 = 0;
        clipX1 = textureWidth - 1;
        clipY1 = textureHeight - 1;
    }

    void Map(int screenX, int screenY, int mapX, int mapY, int width, int height, int transparentColor = -1)
    {
        uint8_t tileWidth = MMU::ReadMem<uint8_t>(MMU::TILE_WIDTH);
        uint8_t tileHeight = MMU::ReadMem<uint8_t>(MMU::TILE_HEIGHT);
        uint8_t mapWidth = MMU::ReadMem<uint8_t>(MMU::MAP_WIDTH);
        uint8_t mapHeight = MMU::ReadMem<uint8_t>(MMU::MAP_HEIGHT);

        for(int tileY = mapY; tileY < mapY + height; tileY++)
        {
            for(int tileX = mapX; tileX < mapX + width; tileX++)
            {
                int tileIndex = MMU::ReadMem<uint8_t>(MMU::MAP_U8 + tileX + tileY * mapWidth);
                int tileBitmapAddress = MMU::TILES_U8 + tileIndex * tileWidth * tileHeight;

                for(int tileMemY = 0; tileMemY < tileHeight; tileMemY++)
                {
                    for(int tileMemX = 0; tileMemX < tileWidth; tileMemX++)
                    {
                        int colorIndex = MMU::ReadMem<uint8_t>(tileBitmapAddress + tileMemX + tileMemY * tileWidth);
                        if(colorIndex == transparentColor)
                            continue;
                        int color = MMU::ReadMem<uint32_t>(MMU::PALETTE_U32 + colorIndex * 4);

                        int pixelCoordX = screenX + (tileX - mapX) * tileWidth + tileMemX;
                        int pixelCoordY = screenY + (tileY - mapY) * tileHeight + tileMemY;

                        Pixel(pixelCoordX, pixelCoordY, colorIndex);
                    }
                }
            }
        }
    }

    void Sprite(int x, int y, int spritex, int spritey, int width, int height)
    {
    }

    void Palette(int bank)
    {
    }

    void Tiles(int bank)
    {
    }

    void Sprite(int bank)
    {
    }

    void PalColor(int index, int r, int g, int b)
    {
        MMU::WriteMem<uint32_t>(MMU::PALETTE_U32 + index * 4, r << 16 | g << 8 | b);
    }

    void Bitmap(int x, int y, int bitmapx, int bitmapy, int width, int height)
    {
    }
}