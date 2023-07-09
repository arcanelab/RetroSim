// RetroSim - Copyright 2011-2023 Zoltán Majoros. All rights reserved.
// https://github.com/arcanelab

#include "GPU.h"
#include "Core.h"
#include <cassert>
#include <stdio.h>

GPU::GPU(Core &core) : core(core)
{
    outputTexture = new uint32_t[textureWidth * textureHeight];
}

void GPU::RenderTileMode()
{
    uint8_t tileMode = core.mmu->ReadMem<uint8_t>(Core::TILE_MODE_U8);

    // TODO: Remove TILE_MODE and make the tile size freely configurable.
    //       This could also possibly increase run-time performance as it would
    //       remove the need for the switch, which has to run every frame.
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

#if 0
// TODO: make outputTexture a parameter
// TODO: use SIMD instrinsics
// Optimized version
void GPU::Render(const uint8_t tileWidth, const uint8_t tileHeight)
{
    const uint_fast8_t numTilesX = width / tileWidth;
    const uint_fast8_t numTilesY = height / tileHeight;
    const uint_fast16_t lastTileMapIndex = numTilesX * numTilesY;
    const uint_fast32_t tilePixelNum = tileWidth * tileHeight;
    uint_fast32_t tileMapAddress = Core::MAP_MEMORY_U8; // as we iterate through the tile map, this address will be incremented

    assert(outputTexture != nullptr);

    for (uint_fast16_t tileMapIndex = 0; tileMapIndex < lastTileMapIndex; tileMapIndex++)
    {
        uint_fast8_t tileIndex = core.mmu->ReadMem<uint8_t>(tileMapAddress);
        const uint_fast32_t tileBitmapAddressBase = Core::TILE_MEMORY_U8 + tileIndex * tilePixelNum;
        // textureOffset will be incremented as we iterate through the tile bitmap
        uint_fast32_t textureOffset = (tileMapIndex % numTilesX * tileWidth) + (tileMapIndex / numTilesX * tileHeight * width);
        uint_fast32_t tileBitmapAddress = tileBitmapAddressBase;

        for (uint_fast8_t y = 0; y < tileHeight; y++)
        {
            for (uint_fast8_t x = 0; x < tileWidth; x++)
            {
                const uint_fast8_t colorIndex = core.mmu->ReadMem<uint8_t>(tileBitmapAddress);
                uint32_t color = core.mmu->ReadMem<uint32_t>(Core::PALETTE_MEMORY_U32 + (colorIndex * 4)); // * 4: one color is 4 bytes
                outputTexture[textureOffset] = color;
                textureOffset++;
                tileBitmapAddress++;
            }

            textureOffset += width - tileWidth;
        }

        tileMapAddress++;
    }
}
#else
void GPU::Render(const uint8_t tileWidth, const uint8_t tileHeight)
{
    const uint8_t numTilesX = textureWidth / tileWidth;
    const uint8_t numTilesY = textureHeight / tileHeight;

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
            tileIndex = core.mmu->ReadMem<uint8_t>(uint32_t(Core::MAP_MEMORY_U8 + tileMapX + tileMapY * (size_t)numTilesX));
            tileBitmapAddress = Core::TILE_MEMORY_U8 + tileIndex * tileWidth * tileHeight;

            for (size_t tileMemY = 0; tileMemY < tileHeight; tileMemY++)
            {
                for (size_t tileMemX = 0; tileMemX < tileWidth; tileMemX++)
                {
                    colorIndex = core.mmu->ReadMem<uint8_t>(uint32_t(tileBitmapAddress + tileMemX + tileMemY * tileWidth));
                    color = core.mmu->ReadMem<uint32_t>(Core::PALETTE_MEMORY_U32 + colorIndex);

                    pixelCoordX = tileMapX * tileWidth + tileMemX;
                    pixelCoordY = tileMapY * tileHeight + tileMemY;

                    outputTexture[pixelCoordX + pixelCoordY * textureWidth] = color;
                }
            }
        }
    }
}
#endif
