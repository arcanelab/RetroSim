// RetroSim - Copyright 2011-2023 Zoltán Majoros. All rights reserved.
// https://github.com/arcanelab

#pragma once

#include <cstdint>

namespace RetroSim::CPUShader
{
    enum FilterMode
    {
        Point,
        Bilinear,
    };

    struct ShaderParams
    {
        uint32_t *inputTexture;
        uint16_t inputWidth;
        uint16_t inputHeight;        
        uint32_t *outputTexture;
        uint16_t outputWidth;
        uint16_t outputHeight;
        FilterMode filterMode;
    };

    void RunShader(const ShaderParams &params);
}
