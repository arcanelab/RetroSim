// RetroSim - Copyright 2011-2023 Zoltán Majoros. All rights reserved.
// https://github.com/arcanelab

#include "GPU.h"
#include "Core.h"
#include <cstring>

Core::Core()
{
    mmu = new MMU(MEMSIZE);
    gpu = new GPU(*this);
}

void Core::Render()
{
    gpu->RenderTileMode();
}
