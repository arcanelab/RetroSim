//
//  core->h
//  Retrosim
//
//  Created by Zoltán Majoros on 09/May/15.
//  Copyright (c) 2015 Zoltán Majoros. All rights reserved.
//

#pragma once

#include "RSTypes.h"

class ICPUInterface;
class SDL_Thread;
class A65000MMU;
class A65000CPU;
class GPU_G1;

struct RSCore
{
    RSCore(int width, int height);
    ~RSCore();
    
    // components
    GPU_G1 *gpu = nullptr;
    A65000MMU *mmu = nullptr;
    A65000CPU *cpu = nullptr;

    // state variables
    int width, height;
    double t0 = 0;

    // local shadow GPU registers
//    GPU_G1::GPURegisters gpuRegisters{};
//    GPU_G1::RGBSurface rgbSurface;
    
    const int GPU_REG_BASE = 0x8000;
    const int SURFACE_PTR  = 0x2000;
    
    uint32 *frameBuffer = nullptr;
    bool isThreadRunning = false;
    SDL_Thread *thread = nullptr;

    void updateFrame(double dt);
    static int tickThread(void *ptr);
    int runCPUThread();
    void stepCPU();
    void stopCPUThread();
};
