#include "MMU.h"
#include <cstdint>

using namespace RetroSim;

namespace RetroSim::API
{
    void PokeU8(uint32_t address, uint8_t value)
    {
        MMU::WriteMem<uint8_t>(address, value);
    }

    void PokeU16(uint32_t address, uint16_t value)
    {
        MMU::WriteMem<uint16_t>(address, value);
    }

    void PokeU32(uint32_t address, uint32_t value)
    {
        MMU::WriteMem<uint32_t>(address, value);
    }
}
