// RetroSim - Copyright 2011-2023 Zoltán Majoros. All rights reserved.
// https://github.com/arcanelab

#pragma once
#include <cstdint>

namespace RetroSim::GPU
{
    const uint_fast16_t textureWidth = 480;
    const uint_fast16_t textureHeight = 256;
    extern uint32_t *outputTexture; // ARGB8888;

    enum TileModes
    {
        TILE_MODE_8x8 = 0,
        TILE_MODE_8x16 = 1,
        TILE_MODE_16x8 = 2,
        TILE_MODE_16x16 = 3
    };

    void RenderTileMode();
};
