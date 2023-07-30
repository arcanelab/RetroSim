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

        // printf("[%d] = %d\n", (int)address.n, (int)value.n);
        // TODO: type checks for address and value

        MMU::WriteMem<uint8_t>(address.n, (uint8_t)value.n);

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
        gravity_class_t *c = gravity_class_new_pair(NULL, "Memory", NULL, 0, 0);
        gravity_class_t *meta = gravity_class_get_meta(c);
        gravity_function_t *pokeu8f = gravity_function_new_internal(NULL, NULL, PokeU8, 0);
        gravity_closure_t *pokeu8c = gravity_closure_new(NULL, pokeu8f);
        gravity_class_bind(meta, "PokeU8", VALUE_FROM_OBJECT(pokeu8c));
        gravity_vm_setvalue(vm, "Memory", VALUE_FROM_OBJECT(c));
    }
}
