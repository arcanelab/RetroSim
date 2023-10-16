// RetroSim - Copyright 2011-2023 Zoltán Majoros. All rights reserved.
// https://github.com/arcanelab

#include "Audio.h"
#include "libcsid.h"
#include "FileUtils.h"
#include "Core.h"

namespace RetroSim::Audio
{
    uint16_t *audioBuffer = nullptr;
    size_t audioBufferSize = 0;
    uint32_t sampleRate = 44100;
    uint8_t *songData = nullptr;

    void Initialize()
    {
        size_t songDataLength;
        // songData = ReadBinaryFile(Core::GetInstance()->GetCoreConfig().GetDataPath() + "/Crimson_Cascade_Groove.sid", songDataLength);
        songData = ReadBinaryFile(Core::GetInstance()->GetCoreConfig().GetDataPath() + "/Turok 2 - Cave Stage.sid", songDataLength);

        libcsid_init(sampleRate * 2, SIDMODEL_8580);
        libcsid_load(songData, songDataLength, 0);
        audioBufferSize = sampleRate / Core::GetInstance()->GetCoreConfig().GetFPS();
        audioBuffer = new uint16_t[audioBufferSize];
    }

    void Shutdown()
    {
        delete[] songData;
        delete[] audioBuffer;
    }

    void RenderAudio()
    {
        libcsid_render((unsigned short *)audioBuffer, audioBufferSize);
    }

    uint16_t *GetAudioBuffer()
    {
        return audioBuffer;
    }

    uint32_t GetAudioBufferSize()
    {
        return audioBufferSize;
    }

    uint32_t GetSampleRate()
    {
        return sampleRate;
    }
}