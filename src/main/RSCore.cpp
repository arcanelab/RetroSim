//
//  RSCore.cpp
//  RetroSim
//
//  Created by Zoltán Majoros on 20/May/2015.
//  Copyright © 2016 Zoltán Majoros. All rights reserved.
//

#include "RSCore.h"
#include "A65000CPU.h"
#include "GPU-G1.h"
#include <SDL2/SDL.h>

RSCore::RSCore(int width, int height) : width(width), height(height)
{
    gpu = new GPU_G1();
    mmu = new A65000MMU();
    cpu = new A65000CPU(mmu);
    cpu->sleep = true;
    
    mmu->write<uint32>(A65000CPU::VEC_RESET, 0x1000);
    mmu->write<uint32>(A65000CPU::VEC_STACKPOINTERINIT, 0x800);
    mmu->write<uint32>(A65000CPU::VEC_HWIRQ, 0x1200);

    GPU_G1::GPURegisters gpuRegisters{};
    GPU_G1::RGBSurface rgbSurface;
    
    // set up graphics mode (in local GPU register copy)
    gpuRegisters.renderMode  = GPU_G1::RenderMode::RM_SCREEN;
    gpuRegisters.numSurfaces = 1;
    gpuRegisters.stageWidth  = (uint16)width;
    gpuRegisters.stageHeight = (uint16)height;
    gpuRegisters.SurfacesPtr = (ptr32)SURFACE_PTR;
    gpuRegisters.currentX = 0; // normally read only, here we just initialize!
    gpuRegisters.currentY = 0;
    
    rgbSurface.width = 64;
    rgbSurface.height = 64;
    rgbSurface.framebufferPtr = 0x3000;
    rgbSurface.xPos = 0;
    rgbSurface.yPos = 100;
    rgbSurface.isHidden = 0;
    rgbSurface.RGBColorType = GPU_G1::RGBColorType::RGB_24BIT;
    
    frameBuffer = new uint32[width*height];
    memset(frameBuffer, 0, (size_t)(width*height*4));
    
    /*
     for(int i=0; i<64*64; i++)
     mmu.write<uint32>(rgbSurface.framebufferPtr + i*4, i*i);
     //mmu.write<uint32>(rgbSurface.gsp.FramebufferPtr + i*4, 0x3f7fcf);
     */

    
    for(int i=0; i<0x100; i++)
        mmu->write<uint8>((uint32)(0x1000+i), (uint8)i);
    
    memcpy(&mmu->memPtr[GPU_REG_BASE], &gpuRegisters, sizeof(gpuRegisters));
    memcpy(&mmu->memPtr[SURFACE_PTR], &rgbSurface, sizeof(rgbSurface));
}

RSCore::~RSCore()
{
    delete [] frameBuffer;
    delete cpu;
    delete gpu;
    delete mmu;
}

void RSCore::updateFrame(double dt)
{
    /*
     rgbSurface.xPos++;
     rgbSurface.yPos++;
     if(rgbSurface.xPos > gpuRegisters.stageWidth / 4)
     {
     rgbSurface.xPos = 10;
     rgbSurface.yPos = 20;
     }
     */
    
    //cpu->tick();
    
    // copy shadow registers into the VM's memory (so that the GPU can read it out)
    //        memcpy(&mmu.memPtr[GPU_REG_BASE], &gpuRegisters, sizeof(gpuRegisters));
    //        memcpy(&mmu.memPtr[SURFACE_PTR], &rgbSurface, sizeof(rgbSurface));
    
    gpu->tick(dt, GPU_REG_BASE, mmu, (uint32 *)frameBuffer, width);
}

// -----------------------------------------------------------

int RSCore::tickThread(void *ptr)
{
    RSCore *self = (RSCore *)ptr;

    while(self->isThreadRunning)
    {
        if(self->cpu->sleep) SDL_Delay(1);// sleep(1); // TODO: how long should we wait?
        self->cpu->tick();
    }

    return 0;
}

// -----------------------------------------------------------

int RSCore::runCPUThread()
{
    if(thread != nullptr) return -1;
    
    SDL_Thread *thread = SDL_CreateThread(tickThread, "RSupdateThread", this);
    if(thread == nullptr)
    {
        puts("Could not initialize thread");
        throw; // TODO: write handler for this
    }

    return 0;
}

// -----------------------------------------------------------

void RSCore::stopCPUThread()
{
    isThreadRunning = false;
    int status;
    SDL_WaitThread(thread, &status);
}

// -----------------------------------------------------------

void RSCore::stepCPU()
{
    cpu->tick();
}
