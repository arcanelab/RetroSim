// RetroSim - Copyright 2011-2023 Zoltán Majoros. All rights reserved.
// https://github.com/arcanelab

#include <cstdint>
#include "MMU.h"
#include "gravity_compiler.h"
#include "gravity_macros.h"
#include "gravity_core.h"
#include "gravity_vm.h"
#include "gravity_vmmacros.h"

using namespace RetroSim;

namespace RetroSim::GravityAPI
{
    bool PokeU8(gravity_vm *vm, gravity_value_t *args, uint16_t nArgs, uint32_t rindex)
    {
        gravity_value_t address = GET_VALUE(1);
        gravity_value_t value = GET_VALUE(2);

        if (VALUE_ISA_FLOAT(address))
            INTERNAL_CONVERT_INT(address, true);
        else if (!VALUE_ISA_INT(address))
            RETURN_ERROR("Address must be an integer.");

        if (VALUE_ISA_STRING(value))
        {
            INTERNAL_CONVERT_STRING(value, true);
            gravity_string_t *s = VALUE_AS_STRING(value);
            // printf("[%x] = %d\n", (int)address.n, s->s[0]);
            MMU::WriteMem<uint8_t>(address.n, (uint8_t)s->s[0]);
        }
        else if (VALUE_ISA_INT(value))
        {
            // printf("[%x] = %d\n", (int)address.n, value.n);
            MMU::WriteMem<uint8_t>(address.n, (uint8_t)value.n);
        }
        else
        {
            RETURN_ERROR("Value must be an integer or a string.");
        }

        RETURN_NOVALUE();
    }

    // bool PokeU16(uint32_t address, uint16_t value)
    // {
    //     MMU::WriteMem<uint16_t>(address, value);
    // }

    // bool PokeU32(uint32_t address, uint32_t value)
    // {
    //     MMU::WriteMem<uint32_t>(address, value);
    // }

    void RegisterAPIFunctions(gravity_vm *vm)
    {
        gravity_gc_setenabled(vm, false);
        // class
        gravity_class_t *c = gravity_class_new_pair(vm, "Memory", NULL, 0, 0);
        gravity_class_t *meta = gravity_class_get_meta(c);

        // method
        gravity_function_t *pokeu8f = gravity_function_new_internal(vm, NULL, PokeU8, 0);
        gravity_closure_t *pokeu8c = gravity_closure_new(vm, pokeu8f);
        gravity_class_bind(meta, "PokeU8", VALUE_FROM_OBJECT(pokeu8c));

        // register class
        gravity_vm_setvalue(vm, "Memory", VALUE_FROM_OBJECT(c));

        gravity_gc_setenabled(vm, true);
    }
}
