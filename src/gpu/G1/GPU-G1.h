/*
 8888888b.          888                             d8b
 888   Y88b         888                             Y8P
 888    888         888
 888   d88P .d88b.  888888 888d888 .d88b.  .d8888b  888 88888b.d88b.
 8888888P" d8P  Y8b 888    888P"  d88""88b 88K      888 888 "888 "88b
 888 T88b  88888888 888    888    888  888 "Y8888b. 888 888  888  888
 888  T88b Y8b.     Y88b.  888    Y88..88P      X88 888 888  888  888
 888   T88b "Y8888   "Y888 888     "Y88P"   88888P' 888 888  888  888

 Created: 02.01.2015
 Copyright (c) 2015 Zoltan Majoros. All rights reserved.
*/

#pragma once

#include "RSTypes.h"
#include "A65000MMU.h"
#include <string>

typedef uint32_t ptr32;
typedef int8_t   bool8;

struct GPU_G1
{
    // --- enumerations ---
    
    enum RenderMode
    {
        RM_SCREEN, RM_LINE, RM_PIXEL
    };

    enum SurfaceType
    {
        SURFACE_NONE,
        SURFACE_RGB,
        SURFACE_PALETTIZED,
        SURFACE_PLANAR,
        SURFACE_TILED
    };

    enum PaletteType
    {
        PALETTE_24BIT,
        PALETTE_12BIT
    };
    
    enum RGBColorType
    {
        RGB_24BIT,
        RGB_12BIT
    };

    // --- structs ---

    #pragma pack(1)
    struct GPURegisters
    {
        uint8  renderMode  = 0; // rw (screen/line/pixel)
        uint16 stageWidth  = 0; // rw
        uint16 stageHeight = 0; // rw
        uint16 currentX    = 0; // r (currently drawn pixel coordinates)
        uint16 currentY    = 0; // r
        uint16 numSurfaces = 0; // rw
        ptr32  SurfacesPtr = 0; // rw
    };
    
    struct GenericSurface
    {
        uint8  type           = 0;
        bool8  isHidden       = 0;
        uint16 width          = 0;
        uint16 height         = 0;
        int32  xPos           = 0; // position in relation to Stage (0,0)
        int32  yPos           = 0;
        ptr32  framebufferPtr = 0;
    };
    
    struct RGBSurface: GenericSurface
    {
        uint8  RGBColorType = 0;
        
        RGBSurface()
        {
            type = SurfaceType::SURFACE_RGB;
        }
    };
    
    struct PalettizedSurface: GenericSurface
    {
        ptr32  PalettePtr  = 0;
        uint8  paletteType = 0;
        
        PalettizedSurface()
        {
            type = SurfaceType::SURFACE_PALETTIZED;
        }
    };
    
    struct TileSurface
    {
        uint8  type           = 0;
        bool8  isHidden       = 0;
        uint16 width          = 0;
        uint16 height         = 0;
        int32  xPos           = 0; // position in relation to Stage (0,0)
        int32  yPos           = 0;
        ptr32  framebufferPtr = 0;
        
        uint8   tileWidth  = 0;  // in pixels
        uint8   tileHeight = 0;
        ptr32   tileSetSurfacePtr = 0; // contains the actual image data for the tiles
        
        TileSurface()
        {
            type = SurfaceType::SURFACE_TILED;
        }
    };

    int ptr = 0;
    double deltas[1000];
    
    // --- public method ---
    void tick(double elapsedTimeInMs, ptr32 gpuRegistersPtr, A65000MMU *mmu, uint32 *destPixels, uint32 pitch)
    {
        this->regs = (GPURegisters *)(mmu->memPtr+gpuRegistersPtr);
        this->dT_ms = elapsedTimeInMs;
        this->mmu = mmu;
        this->destPixels = destPixels;
        this->destPitch = pitch;
        
        deltas[ptr++] = dT_ms;
        if(ptr>=1000) ptr = 0;
        
        render();
    }
    
    std::string getEnumStringRenderMode(RenderMode rm);
    std::string getEnumStringSurfaceType(SurfaceType st);
    std::string getEnumStringPaletteType(PaletteType pt);
    std::string getEnumStringRGBColorType(RGBColorType ct);

private:
    
    // --- variables ---
    double          dT_ms;
    GPURegisters*   regs;
    A65000MMU*      mmu;
    uint32*         destPixels;
    uint32          destPitch;
    
    // --- private methods ---

    void render();
    uint32 clamp(const int value, const uint32 minValue, const uint32 maxValue);
    void renderScreen();
    void renderNextLine();
    void renderNextPixel();
};
