// RetroSim - Copyright 2011-2023 Zoltán Majoros. All rights reserved.
// https://github.com/arcanelab

#pragma once
#include <cstdint>

class Core;

enum TileModes
{
    TILE_MODE_8x8,
    TILE_MODE_8x16,
    TILE_MODE_16x8,
    TILE_MODE_16x16
};

class GPU
{
private:
    Core &core;
    uint32_t *outputTexture; // RGBA

    void Render(const uint8_t tileWidth, const uint8_t tileHeight);

public:
    const uint_fast16_t width = 480;
    const uint_fast16_t height = 256;

    enum TileMode
    {
        _8x8,
        _8x16,
        _16x8,
        _16x16
    };

    GPU(Core &core);
    ~GPU(){};

    void RenderTileMode();
};
