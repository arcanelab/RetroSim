// RetroSim - Copyright 2011-2023 Zoltán Majoros. All rights reserved.
// https://github.com/arcanelab

#include "GPU.h"
#include "Core.h"
#include <cassert>

GPU::GPU(Core &core) : core(core)
{
    outputTexture = new uint32_t[width * height];
}

void GPU::RenderTileMode()
{
    uint8_t tileMode = core.ReadMem<uint8_t>(Core::TILE_MODE_U8);

    switch (tileMode)
    {
    case TILE_MODE_8x8:
        Render(8, 8);
        break;
    case TILE_MODE_8x16:
        Render(8, 16);
        break;
    case TILE_MODE_16x8:
        Render(16, 8);
        break;
    case TILE_MODE_16x16:
        Render(16, 16);
        break;
    }
}

// TODO: make outputTexture a parameter
// Optimized version
void GPU::Render(const uint8_t tileWidth, const uint8_t tileHeight)
{
    const uint_fast8_t numTilesX = width / tileWidth;
    const uint_fast8_t numTilesY = height / tileHeight;

    uint_fast8_t tileIndex;
    uint_fast8_t colorIndex;
    uint32_t color;

    const uint_fast16_t lastTileMapIndex = numTilesX * numTilesY;
    const uint_fast32_t tilePixelNum = tileWidth * tileHeight;

    uint_fast32_t tileMapAddress = Core::MAP_MEMORY_U8; // as we iterate through the tile map, this address will be incremented

    assert(outputTexture != nullptr);

    for (uint_fast16_t tileMapIndex = 0; tileMapIndex < lastTileMapIndex; tileMapIndex++)
    {
        tileIndex = core.ReadMem<uint8_t>(tileMapAddress);

        const uint_fast32_t tileBitmapAddressBase = Core::TILE_MEMORY_U8 + tileIndex * tilePixelNum;

        // textureOffset will be incremented as we iterate through the tile bitmap
        uint_fast32_t textureOffset = (tileMapIndex % numTilesX * tileWidth) + (tileMapIndex / numTilesX * tileHeight * width);
        uint_fast32_t tileBitmapAddress = tileBitmapAddressBase;

        for (uint_fast8_t y = 0; y < tileHeight; y++)
        {
            for (uint_fast8_t x = 0; x < tileWidth; x++)
            {
                colorIndex = core.ReadMem<uint8_t>(tileBitmapAddress);
                color = core.ReadMem<uint32_t>(Core::PALETTE_MEMORY_U32 + (colorIndex << 2)); // colorIndex * 4
                outputTexture[textureOffset] = color;
                tileBitmapAddress++;
            }

            textureOffset += width - tileWidth;
        }

        tileMapAddress++;
    }
}
/*
void GPU::Render(const uint8_t tileWidth, const uint8_t tileHeight)
{
    const uint8_t numTilesX = width / tileWidth;
    const uint8_t numTilesY = height / tileHeight;

    uint8_t tileIndex;
    uint8_t colorIndex;
    uint32_t color;

    size_t pixelCoordX;
    size_t pixelCoordY;

    uint16_t tileBitmapAddress;

    for (size_t tileMapY = 0; tileMapY < numTilesY; tileMapY++)
    {
        for (size_t tileMapX = 0; tileMapX < numTilesX; tileMapX++)
        {
            tileIndex = core->ReadMem<uint8_t>(uint32_t(Core::MAP_MEMORY_U8 + tileMapX + tileMapY * numTilesX));
            tileBitmapAddress = Core::TILE_MEMORY_U8 + tileIndex * tileWidth * tileHeight;

            for (size_t tileMemY = 0; tileMemY < tileHeight; tileMemY++)
            {
                for (size_t tileMemX = 0; tileMemX < tileWidth; tileMemX++)
                {
                    colorIndex = core->ReadMem<uint8_t>(uint32_t(tileBitmapAddress + tileMemX + tileMemY * tileWidth));
                    color = core->ReadMem<uint32_t>(Core::PALETTE_MEMORY_U32 + colorIndex);

                    pixelCoordX = tileMapX * tileWidth + tileMemX;
                    pixelCoordY = tileMapY * tileWidth + tileMemY;

                    texture[pixelCoordX + pixelCoordY * width] = color;
                }
            }
        }
    }
}
*/