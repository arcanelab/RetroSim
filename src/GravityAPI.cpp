// RetroSim - Copyright 2011-2023 Zoltán Majoros. All rights reserved.
// https://github.com/arcanelab

#include <cstdint>
#include <mutex>
#include "Core.h"
#include "MMU.h"
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

        LogPrintf(RETRO_LOG_DEBUG, "Write(%d, %d), value size: %d byte(s)\n", address.n, value.n, sizeof(T));

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

        MMU::WriteMem<T>(address.n, finalValue);

        RETURN_NOVALUE();
    }

    void RegisterAPIFunctions(gravity_vm *vm)
    {
        gravity_gc_setenabled(vm, false);
        // class
        gravity_class_t *c = gravity_class_new_pair(vm, "Memory", NULL, 0, 0);
        gravity_class_t *meta = gravity_class_get_meta(c);

        // method
        gravity_function_t *write8f = gravity_function_new_internal(vm, NULL, Write<uint8_t>, 0);
        gravity_closure_t *writeu8c = gravity_closure_new(vm, write8f);
        gravity_class_bind(meta, "Write8", VALUE_FROM_OBJECT(writeu8c));

        gravity_function_t *write16f = gravity_function_new_internal(vm, NULL, Write<uint16_t>, 0);
        gravity_closure_t *writeu16c = gravity_closure_new(vm, write16f);
        gravity_class_bind(meta, "Write16", VALUE_FROM_OBJECT(writeu16c));

        gravity_function_t *write32f = gravity_function_new_internal(vm, NULL, Write<uint32_t>, 0);
        gravity_closure_t *writeu32c = gravity_closure_new(vm, write32f);
        gravity_class_bind(meta, "Write32", VALUE_FROM_OBJECT(writeu32c));

        // register class
        gravity_vm_setvalue(vm, "Memory", VALUE_FROM_OBJECT(c));

        gravity_gc_setenabled(vm, true);
    }
}
