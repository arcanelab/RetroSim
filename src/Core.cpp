// RetroSim - Copyright 2011-2023 Zoltán Majoros. All rights reserved.
// https://github.com/arcanelab

#include "GPU.h"
#include "Core.h"
#include <cstring>
#include <stdlib.h>
#include <stdio.h>

Core::Core()
{
    printf("Core::Core()\n");

    mmu = new MMU(MEMSIZE);
    gpu = new GPU(*this);

    mmu->WriteMem<uint8_t>(TILE_MODE_U8, GPU::TILE_MODE_8x16);

    int tileWidth = 8;
    int tileHeight = 16;

    // Generate map data
    int numTilesX = gpu->textureWidth / tileWidth;
    int numTilesY = gpu->textureHeight / tileHeight;
    for (int i = 0; i < numTilesX * numTilesY; i++)
    {
        // gpu->outputTexture[i] = (i % gpu->width) | (i << 8) || (i << 16) || (i << 24);
        mmu->WriteMem<uint8_t>(Core::MAP_MEMORY_U8 + i, i % 256);
    }

    // print current directory
    system("dir");

    if (mmu->LoadFile("data/unscii-16.tiledata", Core::TILE_MEMORY_U8) == -1)
    {
        printf("Failed to load file\n");
    }

    // for (int i = 0; i < 256; i++)
    // {
    //     for (int y = 0; y < tileHeight; y++)
    //     {
    //         for (int x = 0; x < tileWidth; x++)
    //         {
    //             // mmu->WriteMem<uint8_t>(Core::TILE_MEMORY_U8 + i * 64 + y * tileWidth + x, i);
    //             // mmu->WriteMem<uint8_t>(Core::TILE_MEMORY_U8 + i * 64 + y * 8 + x, (i % 8 + y) % 256);
    //             mmu->WriteMem<uint8_t>(Core::TILE_MEMORY_U8 + i * tileWidth * tileHeight + y * tileWidth + x, (i * 3) % 33);
    //         }
    //     }
    // }

    // uint32_t palette[] = {0x050403, 0x0e0c0c, 0x2d1b1e, 0x612721, 0xb9451d, 0xf1641f, 0xfca570, 0xffe0b7, 0xffffff, 0xfff089, 0xf8c53a, 0xe88a36, 0xb05b2c, 0x673931, 0x271f1b, 0x4c3d2e, 0x855f39, 0xd39741, 0xf8f644, 0xd5dc1d, 0xadb834, 0x7f8e44, 0x586335, 0x333c24, 0x181c19, 0x293f21, 0x477238, 0x61a53f, 0x8fd032, 0xc4f129, 0xd0ffea, 0x97edca, 0x59cf93, 0x42a459, 0x3d6f43, 0x27412d, 0x14121d, 0x1b2447, 0x2b4e95, 0x2789cd, 0x42bfe8, 0x73efe8, 0xf1f2ff, 0xc9d4fd, 0x8aa1f6, 0x4572e3, 0x494182, 0x7864c6, 0x9c8bdb, 0xceaaed, 0xfad6ff, 0xeeb59c, 0xd480bb, 0x9052bc, 0x171516, 0x373334, 0x695b59, 0xb28b78, 0xe2b27e, 0xf6d896, 0xfcf7be, 0xecebe7, 0xcbc6c1, 0xa69e9a, 0x807b7a, 0x595757, 0x323232, 0x4f342f, 0x8c5b3e, 0xc68556, 0xd6a851, 0xb47538, 0x724b2c, 0x452a1b, 0x61683a, 0x939446, 0xc6b858, 0xefdd91, 0xb5e7cb, 0x86c69a, 0x5d9b79, 0x486859, 0x2c3b39, 0x171819, 0x2c3438, 0x465456, 0x64878c, 0x8ac4c3, 0xafe9df, 0xdceaee, 0xb8ccd8, 0x88a3bc, 0x5e718e, 0x485262, 0x282c3c, 0x464762, 0x696682, 0x9a97b9, 0xc5c7dd, 0xe6e7f0, 0xeee6ea, 0xe3cddf, 0xbfa5c9, 0x87738f, 0x564f5b, 0x322f35, 0x36282b, 0x654956, 0x966888, 0xc090a9, 0xd4b8b8, 0xeae0dd, 0xf1ebdb, 0xddcebf, 0xbda499, 0x886e6a, 0x594d4d, 0x33272a, 0xb29476, 0xe1bf89, 0xf8e398, 0xffe9e3, 0xfdc9c9, 0xf6a2a8, 0xe27285, 0xb25266, 0x64364b, 0x2a1e23};

    // Generate palette
    for (int i = 0; i < 256; i++)
    {
        // mmu->WriteMem<uint32_t>(Core::PALETTE_MEMORY_U32 + i * 4, i << 8 | 0xff);
        mmu->WriteMem<uint32_t>(Core::PALETTE_MEMORY_U32 + i * 4, 0xffc0c0c0); // 0xAARRGGBB
    }
    mmu->WriteMem<uint32_t>(Core::PALETTE_MEMORY_U32, 0xff000000); // 0xAARRGGBB = ARGB8888
}

void Core::Render()
{
    gpu->RenderTileMode();
}
