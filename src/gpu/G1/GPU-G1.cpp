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
 Copyright (c) 2015 Zoltán Majoros. All rights reserved.
*/

#include "GPU-G1.h"
#include <string.h>

void GPU_G1::render()
{
#if 1
    static double tFrame = 16.666666667; // 1.0f / 60.0f;
    static double tLine  = tFrame / regs->stageHeight; //1.0f / (60.0f * regs->stageHeight); // TODO: h/v blanking?
    static double tPixel = tLine / regs->stageWidth; //1.0f / (60.0f * regs->stageHeight * regs->stageWidth);

    switch(regs->renderMode)
    {
        case RM_SCREEN:
            if(dT_ms >= tFrame)
                renderScreen();
            break;
        case RM_LINE: // old-school
            if(dT_ms >= tLine)
                renderNextLine();
            break;
        case RM_PIXEL: // old-school
            if(dT_ms >= tPixel)
                renderNextPixel();
            break;
    }
#endif
}

uint32 GPU_G1::clamp(const int value, const uint32 minValue, const uint32 maxValue)
{
    if(value < minValue)
        return minValue;
    if(value > maxValue)
        return maxValue;
    else return value;
}

void GPU_G1::renderScreen()
{
    GenericSurface *surface = (GenericSurface *)(mmu->memPtr + regs->SurfacesPtr);
    bool surfacesLeft = true;

    memset(destPixels, 0x3f, destPitch * regs->stageHeight * 4);

    while(surfacesLeft)
    {
        switch(surface->type)
        {
            case SURFACE_NONE:
                surfacesLeft = false;
                break;
            case SURFACE_PALETTIZED:
            {
                /*
                     Optimization idea:
                     If the current surface is smaller than the stage, iterate through the surface pixels.
                     Otherwise, iterate through the stage pixels.
                  */
                PalettizedSurface *surface = (PalettizedSurface *)(mmu->memPtr + regs->SurfacesPtr);

                const int bitMultiplier = (surface->paletteType == PALETTE_24BIT) ? 4 : 2;

                for(int stageY = 0; stageY < regs->stageHeight; stageY++)
                {
                    for(int stageX = 0; stageX < regs->stageWidth; stageX++)
                    {
                        const int surfaceX = stageX - surface->xPos;
                        const int surfaceY = stageY - surface->yPos; // calc surface offset

                        uint8 paletteIndex = 0;
                        // clipping
                        if((surfaceX >= 0) && (surfaceX < surface->width) && (surfaceY >= 0) && (surfaceY < surface->height))
                        {
                        //    paletteIndex = mmu->memPtr[gsp->FramebufferPtr + (surfaceX + gsp->width * surfaceY)];
                            paletteIndex = mmu->read<uint8>(surface->framebufferPtr + (surfaceX + surface->width * surfaceY));
                            if(surface->paletteType == PALETTE_24BIT)
                            {
//                                destPixels[stageX + destPitch * stageY] = *(uint32 *)(mmu->memPtr+surface->PalettePtr + (paletteIndex * bitMultiplier)); // write pixel
                                destPixels[stageX + destPitch * stageY] = mmu->read<uint32>(surface->PalettePtr + (paletteIndex * bitMultiplier)); // write pixel
                            }
                            else if(surface->paletteType == PALETTE_12BIT)
                            {
//                                const uint16 color12bit = *(uint16 *)(mmu->memPtr+surface->PalettePtr + (paletteIndex * bitMultiplier));
                                const uint16 color12bit = mmu->read<uint16>(surface->PalettePtr + (paletteIndex * bitMultiplier));

                                const uint8 r24 = (color12bit & 0xf) * 17; // 17 <= 255 / 15
                                const uint8 g24 = ((color12bit & 0xf0) >> 4) * 17;
                                const uint8 b24 = ((color12bit & 0xf00) >> 8) * 17;
                                destPixels[stageX + destPitch * stageY] = b24 | g24 << 8 | r24 << 16; // TODO: is this order right?
                            }
                            else
                            {
                                // TODO: invalid paletteType
                            }
                        }
                    }
                }
            }
                break;
            case SURFACE_PLANAR: // old-school

                break;
            case SURFACE_RGB:
            {
                RGBSurface *rgbSurface = (RGBSurface *)surface;

                const int byteMultiplier = rgbSurface->RGBColorType == RGB_24BIT ? 4 : 2;

                /*
                     Optimization idea:
                     If the current surface is smaller than the stage, iterate through the surface pixels.
                     Otherwise, iterate through the stage pixels.
                  */
                for(int y = 0; y < regs->stageHeight; y++)
                {
                    for(int x = 0; x < regs->stageWidth; x++)
                    {
                        const int surfaceX = x - rgbSurface->xPos;
                        const int surfaceY = y - rgbSurface->yPos; // calc surface offset

                        // clipping
                        if((surfaceX >= 0) && (surfaceX < rgbSurface->width) && (surfaceY >= 0) && (surfaceY < rgbSurface->height))
                        {
//                            uint32 sourcePixel = *(uint32 *)(mmu->memPtr+gsp->FramebufferPtr + (surfaceX + gsp->width * surfaceY) * byteMultiplier); // source pixel
                            uint32 sourcePixel = mmu->read<uint32>(rgbSurface->framebufferPtr + (surfaceX + rgbSurface->width * surfaceY) * byteMultiplier); // source pixel
                            destPixels[x + destPitch * y] = sourcePixel; // write pixel
                        }
                    }
                }
            }
                break;
            case SURFACE_TILED:
            {
                const TileSurface *const tileSurface = (TileSurface *)surface;

                const uint32 screenCoord_TopLeftX = clamp(surface->xPos, 0, regs->stageWidth);
                const uint32 screenCoord_TopLeftY = clamp(surface->yPos, 0, regs->stageHeight);
                const uint32 screenCoord_BottomRightX = clamp(surface->xPos + surface->width * tileSurface->tileWidth, 0, regs->stageWidth);
                const uint32 screenCoord_BottomRightY = clamp(surface->yPos + surface->height * tileSurface->tileHeight, 0, regs->stageHeight);

                if(screenCoord_TopLeftX == screenCoord_BottomRightX)
                    break;

                const GenericSurface *const gspTileSetSurface = (GenericSurface *)&mmu->memPtr[tileSurface->tileSetSurfacePtr];
                switch(gspTileSetSurface->type)
                {
                    case SURFACE_RGB:
                    {
                        const RGBSurface *tileSetSurface = (RGBSurface *)gspTileSetSurface;
                        const int multiplierTileSet = tileSetSurface->RGBColorType == RGB_24BIT ? 4 : 2;

                        for(int y = screenCoord_TopLeftY; y < screenCoord_BottomRightY; y++)
                        {
                            for(int x = screenCoord_TopLeftX; x < screenCoord_BottomRightX; x++)
                            {
                                const uint32 tileSurfacePixelX = x - tileSurface->xPos;
                                const uint32 tileSurfacePixelY = y - tileSurface->yPos;
                                const uint32 tileCoordX = tileSurfacePixelX / tileSurface->tileWidth;
                                const uint32 tileCoordY = tileSurfacePixelY / tileSurface->tileHeight;
                                const uint32 tilePixelOffsetX = tileSurfacePixelX % tileSurface->tileWidth;
                                const uint32 tilePixelOffsetY = tileSurfacePixelY % tileSurface->tileHeight;

                                //const uint8 tileIndex = mmu->memPtr[tileSurface->gsp.FramebufferPtr + tileCoordX + tileCoordY * tileSurface->gsp.width];
                                const uint8 tileIndex = mmu->read<uint8>(tileSurface->framebufferPtr + tileCoordX + tileCoordY * tileSurface->width);
                                const uint32 tilePixelOffset = tileSurface->tileWidth * (tileIndex % tileSetSurface->width) + tilePixelOffsetX \
                                + (tilePixelOffsetY + tileSurface->tileWidth * tileIndex / tilePixelOffsetX) / tileSetSurface->width;

                                uint32 color = 0;

                                switch(tileSetSurface->RGBColorType)
                                {
                                    case RGB_24BIT:
                                        //color = *(uint32 *)(mmu->memPtr+tileSetSurface->gsp.FramebufferPtr + tilePixelOffset * multiplierTileSet);
                                        color = mmu->read<uint32>(tileSetSurface->framebufferPtr + tilePixelOffset * multiplierTileSet);
                                        break;
                                    case RGB_12BIT:
                                    {
                                        //uint16 color12bit = *(uint16 *)(mmu->memPtr+tileSetSurface->gsp.FramebufferPtr + tilePixelOffset * multiplierTileSet);
                                        uint16 color12bit = mmu->read<uint16>(tileSetSurface->framebufferPtr + tilePixelOffset * multiplierTileSet);
                                        const uint8 r24 = (color12bit & 0xf) * 17; // 17 <= 255 / 15
                                        const uint8 g24 = ((color12bit & 0xf0) >> 4) * 17;
                                        const uint8 b24 = ((color12bit & 0xf00) >> 8) * 17;
                                        color = b24 | g24 << 8 | r24 << 16; // TODO: is this order right?
                                    }
                                        break;
                                    default:
                                        // TODO: error?
                                        break;
                                }
                                destPixels[x + screenCoord_TopLeftX + (y + screenCoord_TopLeftY) * destPitch] = color;
                            }
                        }
                    }
                        break;
                    default:
                        surfacesLeft = false;
                }
            }
                break;
            default:
                break;
        }
        surface += 0x100;
    }
}

void GPU_G1::renderNextLine() // old-school
{

}

void GPU_G1::renderNextPixel() // old-school
{

}

std::string GPU_G1::getEnumStringRenderMode(RenderMode rm)
{
    switch (rm) {
        case RM_LINE:
            return std::string("Line");
        case RM_PIXEL:
            return std::string("Pixel");
        case RM_SCREEN:
            return std::string("Screen");
    }
}

std::string GPU_G1::getEnumStringSurfaceType(SurfaceType st)
{
    switch(st)
    {
        case SURFACE_NONE:
            return std::string("Empty");
        case SURFACE_RGB:
            return std::string("RGB");
        case SURFACE_PALETTIZED:
            return std::string("Palettized");
        case SURFACE_PLANAR:
            return std::string("Planar");
        case SURFACE_TILED:
            return std::string("Tiled");
        default:
            return std::string("INVALID");
    }
}

std::string GPU_G1::getEnumStringPaletteType(PaletteType pt)
{
    switch(pt)
    {
        case PALETTE_12BIT:
            return std::string("12-bit");
        case PALETTE_24BIT:
            return std::string("24-bit");
    }
}

std::string GPU_G1::getEnumStringRGBColorType(RGBColorType ct)
{
    switch(ct)
    {
        case RGB_12BIT:
            return std::string("12-bit");
        case RGB_24BIT:
            return std::string("24-bit");
    }
}
