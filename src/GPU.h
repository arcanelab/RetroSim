// RetroSim - Copyright 2011-2023 Zoltán Majoros. All rights reserved.
// https://github.com/arcanelab

#pragma once
#include <cstdint>

class Core;

class GPU
{
private:
    Core &core;

    void Render(const uint8_t tileWidth, const uint8_t tileHeight);

public:
    uint32_t *outputTexture; // ARGB8888
    const uint_fast16_t width = 480;
    const uint_fast16_t height = 256;

    enum TileModes
    {
        TILE_MODE_8x8 = 0,
        TILE_MODE_8x16 = 1,
        TILE_MODE_16x8 = 2,
        TILE_MODE_16x16 = 3
    };

    GPU(Core &core);
    ~GPU(){};

    void RenderTileMode();
};
