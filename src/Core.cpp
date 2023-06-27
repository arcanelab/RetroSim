// RetroSim - Copyright 2011-2023 Zoltán Majoros. All rights reserved.
// https://github.com/arcanelab

#include "Core.h"
#include "GPU.h"
#include <cstring>

Core::Core()
{
    memory = new uint8_t[MEMSIZE];
    memset(memory, 0, MEMSIZE);

    gpu = new GPU(*this);
}
