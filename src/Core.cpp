// RetroSim - Copyright 2011-2023 Zoltán Majoros. All rights reserved.
// https://github.com/arcanelab

#include "GPU.h"
#include "Core.h"
#include "MMU.h"
#include <cstring>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

namespace RetroSim
{
    RetroSim::Core *RetroSim::Core::instance = nullptr;

    uint32_t palette[64] = {0xff060608, 0xff141013, 0xff3b1725, 0xff73172d, 0xffb4202a, 0xffdf3e23, 0xfffa6a0a, 0xfff9a31b, 0xffffd541, 0xfffffc40, 0xffd6f264, 0xff9cdb43, 0xff59c135, 0xff14a02e, 0xff1a7a3e, 0xff24523b, 0xff122020, 0xff143464, 0xff285cc4, 0xff249fde, 0xff20d6c7, 0xffa6fcdb, 0xffffffff, 0xfffef3c0, 0xfffad6b8, 0xfff5a097, 0xffe86a73, 0xffbc4a9b, 0xff793a80, 0xff403353, 0xff242234, 0xff221c1a, 0xff322b28, 0xff71413b, 0xffbb7547, 0xffdba463, 0xfff4d29c, 0xffdae0ea, 0xffb3b9d1, 0xff8b93af, 0xff6d758d, 0xff4a5462, 0xff333941, 0xff422433, 0xff5b3138, 0xff8e5252, 0xffba756a, 0xffe9b5a3, 0xffe3e6ff, 0xffb9bffb, 0xff849be4, 0xff588dbe, 0xff477d85, 0xff23674e, 0xff328464, 0xff5daf8d, 0xff92dcba, 0xffcdf7e2, 0xffe4d2aa, 0xffc7b08b, 0xffa08662, 0xff796755, 0xff5a4e44, 0xff423934};

    void Core::Initialize(const std::string &basePath)
    {
        coreConfig.Initialize(basePath);
        GPU::Initialize();

        LoadFont();

        int tileWidth = 8;
        int tileHeight = 16;

        // Generate palette
        for (int i = 0; i < 256; i++)
        {
            MMU::WriteMem<uint32_t>(MMU::PALETTE_U32 + i * 4, palette[i % 64]); // 0xAARRGGBB
        }

        int numTiles = (GPU::textureWidth / tileWidth) * (GPU::textureHeight / tileHeight);
        for(int i=0; i<numTiles; i++)
        {
            MMU::WriteMem<uint8_t>(MMU::MAP_U8 + i, i % 256);
        }
    }

    void Core::LoadFont()
    {
        // print current directory
        char basePath[1024];
        getcwd(basePath, 1024);
        printf("Current directory: %s\n", basePath);
        
        std::string finalPath = coreConfig.GetDataPath() + "/unscii-16.tiledata";
        MMU::LoadFile(finalPath.c_str(), MMU::CHARSET);

        // copy first 16K from character ram to tile ram
        for(int i=0; i<0x4000; i++)
        {
            uint8_t value = MMU::ReadMem<uint8_t>(MMU::CHARSET + i);
            MMU::WriteMem<uint8_t>(MMU::TILES_U8+i, value);         
        }
    }

    CoreConfig Core::GetCoreConfig()
    {
        return coreConfig;
    }

    bool Core::LoadCartridge(const std::string &path)
    {
        return true;
    }

    void Core::RunNextFrame()
    {
        GPU::frameNumber++;
        // RetroSim::GPU::RenderTileMode();

        // Test all GPU functions. The color parameter is an index into a 256-long palette.
        // All drawings must be visible on the screen of 480x256 pixels.
        // GPU::Cls();
        GPU::Clip(50, 50, 480-50, 256-50);
        GPU::Rect(0, 0, 479, 255, 0, true);
        GPU::Line(0, 0, 479, 255, 1);
        GPU::Circle(480/2, 256/2, 128, 2, false);
        GPU::Circle(240, 128, 50, 3, true);
        GPU::Rect(100, 100, 100, 100, 4, false);
        GPU::NoClip();
        GPU::Rect(100, 100, 50, 50, 5, true);
        GPU::Tri(200, 200, 300, 200, 250, 100, 6, false);
        GPU::Tri(200, 200, 300, 200, 250, 100, 7, true);
        GPU::Tex(0, 0, 100, 0, 0, 100, 0, 0, 100, 0, 0, 100);
        GPU::Pixel(200, 200, 8);
        
        GPU::Clip(100, 150, 480-150, 256-150);
        GPU::Cls();
        GPU::NoClip();

        srand(GPU::frameNumber / 20);
        int radius = rand() % 40 + 10;

        int colorIndex = (GPU::frameNumber / 20) % 64;

        static int x = 0;
        static int y = 0;
        static int dx = 1;
        static int dy = 1;
        x += dx;
        y += dy;
        if (x < 0 || x > 480)
            dx = -dx;
        if (y < 0 || y > 256)
            dy = -dy;
        GPU::Circle(x, y, radius, colorIndex, true);

        // GPU::Cls();
        GPU::Print("RetroSim", (GPU::textureWidth - GPU::frameNumber) % GPU::textureWidth, 150, colorIndex, 0, 1);
        GPU::Map(GPU::frameNumber % GPU::textureWidth, 40, 0, 0, 30, 5, 0);
    }

    void Core::Reset()
    {
    }
}
