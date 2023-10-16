// RetroSim - Copyright 2011-2023 Zoltán Majoros. All rights reserved.
// https://github.com/arcanelab

#pragma once

#include <cstdint>
#include <cstddef>

namespace RetroSim::Audio
{
    void Initialize();
    void Shutdown();
    void RenderAudio();
    uint16_t *GetAudioBuffer();
    uint32_t GetAudioBufferSize();
    uint32_t GetSampleRate();
};
