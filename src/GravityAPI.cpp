// RetroSim - Copyright 2011-2023 Zoltán Majoros. All rights reserved.
// https://github.com/arcanelab

#include <cstdint>
#include <mutex>
#include "Core.h"
#include "MMU.h"
#include "GPU.h"
#include "gravity_compiler.h"
#include "gravity_macros.h"
#include "gravity_core.h"
#include "gravity_vm.h"
#include "gravity_vmmacros.h"

using namespace RetroSim;

namespace RetroSim::GravityAPI
{
    template <typename T>
    bool Write(gravity_vm *vm, gravity_value_t *args, uint16_t nArgs, uint32_t rindex)
    {
        static_assert(std::is_integral<T>::value, "T must be an integral type.");

        std::lock_guard<std::mutex> lock(Core::GetInstance()->memoryMutex);

        gravity_value_t address = GET_VALUE(1);
        gravity_value_t value = GET_VALUE(2);

        if VALUE_ISA_FLOAT (address)
            INTERNAL_CONVERT_INT(address, true);
        else if (!VALUE_ISA_INT(address))
            RETURN_ERROR("Address must be an integer.");

        // LogPrintf(RETRO_LOG_DEBUG, "Write(%d, %d), value size: %d byte(s)\n", address.n, value.n, sizeof(T));

        T finalValue = 0;

        if VALUE_ISA_STRING (value)
        {
            INTERNAL_CONVERT_STRING(value, true);
            gravity_string_t *s = VALUE_AS_STRING(value);
            finalValue = (T)s->s[0];
        }
        else if VALUE_ISA_INT (value)
        {
            finalValue = (T)value.n;
        }
        else
        {
            RETURN_ERROR("Value must be an integer or a string.");
        }

        if (value.n > std::numeric_limits<T>::max())
            RETURN_ERROR("Value is too large to fit into the specified type.");

        if (address.n > MMU::memorySize - sizeof(T))
            RETURN_ERROR("Address is out of bounds.");

        MMU::WriteMem<T>(address.n, finalValue);

        RETURN_NOVALUE();
    }

    template <typename T>
    bool Read(gravity_vm *vm, gravity_value_t *args, uint16_t nArgs, uint32_t rindex)
    {
        static_assert(std::is_integral<T>::value, "T must be an integral type.");

        if (nArgs != 2)
            RETURN_ERROR("Read() expects 1 argument.");

        std::lock_guard<std::mutex> lock(Core::GetInstance()->memoryMutex);

        gravity_value_t address = GET_VALUE(1);

        if VALUE_ISA_FLOAT (address)
            INTERNAL_CONVERT_INT(address, true);
        else if (!VALUE_ISA_INT(address))
            RETURN_ERROR("Address must be an integer.");

        int addressValue = (int)VALUE_AS_INT(args[1]);

        // LogPrintf(RETRO_LOG_DEBUG, "Read(%d), value size: %d byte(s)\n", addressValue, sizeof(T));

        T value = MMU::ReadMem<T>(addressValue);

        RETURN_VALUE(VALUE_FROM_INT(MMU::ReadMem<T>(addressValue)), rindex);
        return true;
    }

    bool MemoryPropertyGetter(gravity_vm *vm, gravity_value_t *args, uint16_t nArgs, uint32_t rindex)
    {
#pragma unused(args, nargs)
        gravity_value_t key = GET_VALUE(1);
        if (!VALUE_ISA_STRING(key))
            RETURN_VALUE(VALUE_FROM_NULL, rindex);

        if (strcmp(VALUE_AS_CSTRING(key), "MEMORY_SIZE") == 0)
        {
            RETURN_VALUE(VALUE_FROM_INT(MMU::memorySize), rindex);
        }
        if (strcmp(VALUE_AS_CSTRING(key), "PALETTE_U32") == 0)
        {
            RETURN_VALUE(VALUE_FROM_INT(MMU::PALETTE_U32), rindex);
        }
        if (strcmp(VALUE_AS_CSTRING(key), "MAP_U8") == 0)
        {
            RETURN_VALUE(VALUE_FROM_INT(MMU::MAP_U8), rindex);
        }
        if (strcmp(VALUE_AS_CSTRING(key), "TILES_U8") == 0)
        {
            RETURN_VALUE(VALUE_FROM_INT(MMU::TILES_U8), rindex);
        }
        if (strcmp(VALUE_AS_CSTRING(key), "SPRITE_ATLAS_U8") == 0)
        {
            RETURN_VALUE(VALUE_FROM_INT(MMU::SPRITE_ATLAS_U8), rindex);
        }
        if (strcmp(VALUE_AS_CSTRING(key), "BITMAP_U8") == 0)
        {
            RETURN_VALUE(VALUE_FROM_INT(MMU::BITMAP_U8), rindex);
        }
        if (strcmp(VALUE_AS_CSTRING(key), "CHARSET_U8") == 0)
        {
            RETURN_VALUE(VALUE_FROM_INT(MMU::CHARSET_U8), rindex);
        }
    }

    static bool MemoryPropertySetter(gravity_vm *vm, gravity_value_t *args, uint16_t nargs, uint32_t rindex)
    {
        RETURN_ERROR("This property is read-only.");
        RETURN_NOVALUE();
    }

    bool GPUPropertyGetter(gravity_vm *vm, gravity_value_t *args, uint16_t nArgs, uint32_t rindex)
    {
#pragma unused(args, nargs)
        gravity_value_t key = GET_VALUE(1);
        if (!VALUE_ISA_STRING(key))
            RETURN_VALUE(VALUE_FROM_NULL, rindex);

        if (strcmp(VALUE_AS_CSTRING(key), "SCREEN_WIDTH") == 0)
        {
            RETURN_VALUE(VALUE_FROM_INT(MMU::memory.gpu.screenWidth), rindex);
        }
        if (strcmp(VALUE_AS_CSTRING(key), "SCREEN_HEIGHT") == 0)
        {
            RETURN_VALUE(VALUE_FROM_INT(MMU::memory.gpu.screenHeight), rindex);
        }
        if (strcmp(VALUE_AS_CSTRING(key), "TILE_WIDTH") == 0)
        {
            RETURN_VALUE(VALUE_FROM_INT(MMU::memory.gpu.tileWdith), rindex);
        }
        if (strcmp(VALUE_AS_CSTRING(key), "TILE_HEIGHT") == 0)
        {
            RETURN_VALUE(VALUE_FROM_INT(MMU::memory.gpu.tileHeight), rindex);
        }
        if (strcmp(VALUE_AS_CSTRING(key), "MAP_WIDTH") == 0)
        {
            RETURN_VALUE(VALUE_FROM_INT(MMU::memory.gpu.mapWidth), rindex);
        }
        if (strcmp(VALUE_AS_CSTRING(key), "MAP_HEIGHT") == 0)
        {
            RETURN_VALUE(VALUE_FROM_INT(MMU::memory.gpu.mapHeight), rindex);
        }
        if (strcmp(VALUE_AS_CSTRING(key), "SPRITE_ATLAS_PITCH") == 0)
        {
            RETURN_VALUE(VALUE_FROM_INT(MMU::memory.gpu.spriteAtlasPitch), rindex);
        }
        if (strcmp(VALUE_AS_CSTRING(key), "BITMAP_PITCH") == 0)
        {
            RETURN_VALUE(VALUE_FROM_INT(MMU::memory.gpu.screenWidth), rindex);
        }
        if (strcmp(VALUE_AS_CSTRING(key), "CHARACTER_COLOR") == 0)
        {
            RETURN_VALUE(VALUE_FROM_INT(MMU::memory.generalRegisters.currentFPS), rindex);
        }
        if (strcmp(VALUE_AS_CSTRING(key), "FIXED_FRAME_TIME") == 0)
        {
            RETURN_VALUE(VALUE_FROM_INT(MMU::memory.generalRegisters.fixedFrameTime), rindex);
        }
        if (strcmp(VALUE_AS_CSTRING(key), "DELTA_TIME") == 0)
        {
            RETURN_VALUE(VALUE_FROM_INT(MMU::memory.generalRegisters.deltaTime), rindex);
        }
        if (strcmp(VALUE_AS_CSTRING(key), "FRAME_COUNTER") == 0)
        {
            RETURN_VALUE(VALUE_FROM_INT(MMU::memory.generalRegisters.frameCounter), rindex);
        }
        if (strcmp(VALUE_AS_CSTRING(key), "REFRESH_RATE") == 0)
        {
            RETURN_VALUE(VALUE_FROM_INT(MMU::memory.generalRegisters.refreshRate), rindex);
        }
        if (strcmp(VALUE_AS_CSTRING(key), "CURRENT_FPS") == 0)
        {
            RETURN_VALUE(VALUE_FROM_INT(MMU::memory.generalRegisters.currentFPS), rindex);
        }        
    }

    static bool GPUPropertySetter(gravity_vm *vm, gravity_value_t *args, uint16_t nargs, uint32_t rindex)
    {
        RETURN_ERROR("This property is read-only.");
        RETURN_NOVALUE();
    }

    bool Cls(gravity_vm *vm, gravity_value_t *args, uint16_t nArgs, uint32_t rindex)
    {
        int colorIndexValue = 0;
        if (nArgs == 2)
        {
            gravity_value_t colorIndex = GET_VALUE(1);

            if VALUE_ISA_FLOAT (colorIndex)
                INTERNAL_CONVERT_INT(colorIndex, true);
            else if (!VALUE_ISA_INT(colorIndex))
                RETURN_ERROR("Color must be an integer.");

            colorIndexValue = (int)VALUE_AS_INT(args[1]);
        }
        if (nArgs > 2)
        {
            RETURN_ERROR("Invalid amount of arguments. Cls takes zero or a single optional argument: color index.");
        }

        GPU::ClearScreen(colorIndexValue);
        RETURN_NOVALUE();
    }

    void RegisterMemoryAPI(gravity_vm *vm)
    {
        gravity_gc_setenabled(vm, false);
        // class
        gravity_class_t *c = gravity_class_new_pair(vm, "Memory", NULL, 0, 0);
        gravity_class_t *meta = gravity_class_get_meta(c);

        // methods
        gravity_function_t *write8f = gravity_function_new_internal(vm, NULL, Write<uint8_t>, 0);
        gravity_closure_t *writeu8c = gravity_closure_new(vm, write8f);
        gravity_class_bind(meta, "Write8", VALUE_FROM_OBJECT(writeu8c));

        gravity_function_t *write16f = gravity_function_new_internal(vm, NULL, Write<uint16_t>, 0);
        gravity_closure_t *write16c = gravity_closure_new(vm, write16f);
        gravity_class_bind(meta, "Write16", VALUE_FROM_OBJECT(write16c));

        gravity_function_t *write32f = gravity_function_new_internal(vm, NULL, Write<uint32_t>, 0);
        gravity_closure_t *write32c = gravity_closure_new(vm, write32f);
        gravity_class_bind(meta, "Write32", VALUE_FROM_OBJECT(write32c));

        gravity_function_t *read8f = gravity_function_new_internal(vm, NULL, Read<uint8_t>, 0);
        gravity_closure_t *read8c = gravity_closure_new(vm, read8f);
        gravity_class_bind(meta, "Read8", VALUE_FROM_OBJECT(read8c));

        gravity_function_t *read16f = gravity_function_new_internal(vm, NULL, Read<uint16_t>, 0);
        gravity_closure_t *read16c = gravity_closure_new(vm, read16f);
        gravity_class_bind(meta, "Read16", VALUE_FROM_OBJECT(read16c));

        gravity_function_t *read32f = gravity_function_new_internal(vm, NULL, Read<uint32_t>, 0);
        gravity_closure_t *read32c = gravity_closure_new(vm, read32f);
        gravity_class_bind(meta, "Read32", VALUE_FROM_OBJECT(read32c));

        // read-only properties
        gravity_closure_t *closure = computed_property_create(NULL, NEW_FUNCTION(MemoryPropertyGetter), NEW_FUNCTION(MemoryPropertySetter));
        gravity_value_t value = VALUE_FROM_OBJECT(closure);
        gravity_class_bind(meta, "MEMORY_SIZE", value);
        gravity_class_bind(meta, "PALETTE_U32", value);
        gravity_class_bind(meta, "MAP_U8", value);
        gravity_class_bind(meta, "TILES_U8", value);
        gravity_class_bind(meta, "SPRITE_ATLAS_U8", value);
        gravity_class_bind(meta, "BITMAP_U8", value);
        gravity_class_bind(meta, "CHARSET_U8", value);

        // register class
        gravity_vm_setvalue(vm, "Memory", VALUE_FROM_OBJECT(c));

        gravity_gc_setenabled(vm, true);
    }

    void RegisterGPUFunctions(gravity_vm *vm)
    {
        gravity_gc_setenabled(vm, false);
        // class
        gravity_class_t *c = gravity_class_new_pair(vm, "GPU", NULL, 0, 0);
        gravity_class_t *meta = gravity_class_get_meta(c);

        // methods
        gravity_function_t *clscolorf = gravity_function_new_internal(vm, NULL, Cls, 0);
        gravity_closure_t *clscolorc = gravity_closure_new(vm, clscolorf);
        gravity_class_bind(meta, "Cls", VALUE_FROM_OBJECT(clscolorc));

        // properties
        gravity_closure_t *closure = computed_property_create(NULL, NEW_FUNCTION(GPUPropertyGetter), NEW_FUNCTION(GPUPropertySetter));
        gravity_value_t value = VALUE_FROM_OBJECT(closure);
        gravity_class_bind(meta, "SCREEN_WIDTH", value);
        gravity_class_bind(meta, "SCREEN_HEIGHT", value);
        gravity_class_bind(meta, "TILE_WIDTH", value);
        gravity_class_bind(meta, "TILE_HEIGHT", value);
        gravity_class_bind(meta, "MAP_WIDTH", value);
        gravity_class_bind(meta, "MAP_HEIGHT", value);
        gravity_class_bind(meta, "SPRITE_ATLAS_PITCH", value);
        gravity_class_bind(meta, "BITMAP_PITCH", value);
        gravity_class_bind(meta, "CHARACTER_COLOR", value);
        gravity_class_bind(meta, "FIXED_FRAME_TIME", value);
        gravity_class_bind(meta, "DELTA_TIME", value);
        gravity_class_bind(meta, "FRAME_COUNTER", value);
        gravity_class_bind(meta, "REFRESH_RATE", value);
        gravity_class_bind(meta, "CURRENT_FPS", value);        

        // register class
        gravity_vm_setvalue(vm, "GPU", VALUE_FROM_OBJECT(c));

        gravity_gc_setenabled(vm, true);
    }

    void RegisterAPIFunctions(gravity_vm *vm)
    {
        RegisterMemoryAPI(vm);
        RegisterGPUFunctions(vm);
    }
}
