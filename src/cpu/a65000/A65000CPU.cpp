/*
 8888888b.          888                             d8b
 888   Y88b         888                             Y8P
 888    888         888
 888   d88P .d88b.  888888 888d888 .d88b.  .d8888b  888 88888b.d88b.
 8888888P" d8P  Y8b 888    888P"  d88""88b 88K      888 888 "888 "88b
 888 T88b  88888888 888    888    888  888 "Y8888b. 888 888  888  888
 888  T88b Y8b.     Y88b.  888    Y88..88P      X88 888 888  888  888
 888   T88b "Y8888   "Y888 888     "Y88P"   88888P' 888 888  888  888

 Created on 11/26/13.
 Copyright (c) 2013 Zoltán Majoros. All rights reserved.
*/
#include "A65000CPU.h"
#include "A65000MMU.h"
#include <cassert>
#include <type_traits>
#include <limits>

// ==============================================================================
// === ICPUInterface method implementations =====================================
// ==============================================================================

void A65000CPU::interruptRaised(bool isNMI)
{
    sleep = false;

    if(!isNMI && statusRegister.i)
        return;
    
    SP--; // save the status register to the stack
    mmu->write<uint8>(SP, *(uint8 *)(&statusRegister));
    
    SP -= 4; // save the PC to the stack
    mmu->write<uint32>(SP, PC);
    
    if(isNMI)
        PC = mmu->read<uint32>(VEC_NMI); // jump to the NMI-handler
    else
        PC = mmu->read<uint32>(VEC_HWIRQ); // jump to the IRQ-handler
}

// ==============================================================================

void A65000CPU::reset()
{
    sleep = false;
    
    for(int i=0; i<14; i++)
        registers[i] = 0;
    
    PC = mmu->read<uint32>(VEC_RESET);
    SP = mmu->read<uint32>(VEC_STACKPOINTERINIT);
}

void A65000CPU::setPC(unsigned int newPC)
{
    PC = (uint32)newPC;
}

// ==============================================================================

int A65000CPU::tick() // return value: the number of cycles used
{
    if(sleep)
        return 1;
    
    try
    {
        return runNextInstruction();
    }
    catch (A65000Exception exception)
    {
        switch (exception.type)
        {
            case EX_INVALID_INSTRUCTION:
                PC = mmu->read<uint32>(VEC_ILLEGALINSTRUCTION);
                break;
            case EX_INVALID_ADDRESS:
                PC = mmu->read<uint32>(VEC_ILLEGALADDRESS);
                break;
            case EX_DIVISION_BY_ZERO:
                PC = mmu->read<uint32>(VEC_DIVZERO);
                break;
            default:
                // TODO: here?
                break;
        }
    }
    
    return 2;
}

// ==============================================================================
// === private class methods ====================================================
// ==============================================================================

int A65000CPU::runNextInstruction()
{
    const InstructionWord &instr = fetchInstructionWord();
    
    switch(instr.opcodeSize)
    {
        case OS_8BIT:  return decodeInstruction<uint8 >(instr);
        case OS_16BIT: return decodeInstruction<uint16>(instr);
        case OS_32BIT: return decodeInstruction<uint32>(instr);
        default: throw A65000Exception(EX_INVALID_INSTRUCTION);
    }    
}

//==============================================================================

template<class T>
int A65000CPU::decodeInstruction(const InstructionWord &instr)
{
    switch(instr.addressingMode)
    {
        case AM_REG_IMMEDIATE: return handleAddressingMode_RegisterImmediate<T>(instr);              // sub.w r0, 20
        case AM_REGISTER2: return handleAddressingMode_Register2<T>(instr);                          // sub.w r0, r1
        case AM_ABSOLUTE_SRC: return handleAddressingMode_AbsoluteSrc<T>(instr);                     // sub.w r0, [$a000]
        case AM_ABSOLUTE_DEST: return handleAddressingMode_AbsoluteDst<T>(instr);                    // sub.w [$a000], r0
        case AM_REGISTER_INDIRECT_SRC: return handleAddressingMode_RegisterIndirectSrc<T>(instr);    // sub.w r0, [r1]
        case AM_REGISTER_INDIRECT_DEST: return handleAddressingMode_RegisterIndirectDst<T>(instr);   // sub.w [r0], r1
        case AM_INDEXED_SRC: return handleAddressingMode_IndexedSrc<T>(instr);                       // sub.w r0, [$1000+r1]
        case AM_INDEXED_DEST: return handleAddressingMode_IndexedDst<T>(instr);                      // sub.w [$1000+r0], r1
        case AM_REGISTER_INDIRECT1: return handleAddressingMode_RegisterIndirect<T>(instr);          // inc.w [r1]-
        case AM_INDEXED1: return handleAddressingMode_Indexed1<T>(instr);                            // inc.w [r1 + $200]
        case AM_REGISTER1: return handleAddressingMode_Register1<T>(instr);                          // inc.w r1
        case AM_DIRECT: return handleAddressingMode_Direct(instr);                                   // jmp $100
        case AM_CONST_IMMEDIATE: return handleAddressingMode_ConstImmediate<T>(instr);               // push.w $330
        case AM_ABSOLUTE1: return handleAddressingMode_Absolute1<T>(instr);                          // inc.w [$200]
        case AM_RELATIVE: return handleAddressingMode_Relative<T>(instr);                            // bne -50
        case AM_NONE: return handleAddressingMode_Implied(instr);                                    // rts, cli, etc
        default: throw A65000Exception(EX_INVALID_INSTRUCTION);
    }
}

//==============================================================================

int A65000CPU::handleAddressingMode_Implied(const InstructionWord &inst)
{
    int cycles = 1;
    
    switch(inst.instructionCode)
    {
        case I_NOP: break;
        case I_SEI: statusRegister.i = 1; break;
        case I_CLI: statusRegister.i = 0; break;
        case I_SEC: statusRegister.c = 1; break;
        case I_CLC: statusRegister.c = 0; break;
        case I_SEV: statusRegister.v = 1; break;
        case I_CLV: statusRegister.v = 0; break;

        case I_RTS:
            PC = mmu->read<uint32>(SP); // POP PC
            SP += 4;
            cycles = 3;
            break;
        
        case I_PUSHA:
            for(int i=0; i<16; i++)
            {
                SP -= 4;
                mmu->write<uint32>(SP, registers[i]);
            }
            cycles = 32;
            break;
        
        case I_POPA:
            for(int i=0; i<16; i++)
            {
                registers[i] = mmu->read<uint32>(SP);
                SP += 4;
            }
            cycles = 32;
            break;
            
        case I_BRK:
            if(statusRegister.i)
                return 1;
            
            SP--;
            mmu->write<uint8>(SP, *(uint8 *)(&statusRegister)); // PUSH Status

            SP -= 4;
            mmu->write<uint32>(SP, PC); // PUSH PC

            PC = mmu->read<uint32>(VEC_SOFTIRQ); // JMP [VEC_SOFTIRQ]
            
            statusRegister.b = 1;
            
            cycles = 6;
            break;
            
        case I_RTI:
            PC = mmu->read<uint32>(SP); // POP PC
            SP += 4;
            *(uint8*)(&statusRegister) = (uint8)mmu->read<uint8>(SP); // POP Status
            SP++;
            cycles = 5;
            break;
            
        case I_SLP:
            sleep = true;
            break;
            
        default: throw A65000Exception(EX_INVALID_INSTRUCTION);
    }
    
    return cycles;
}

//==============================================================================

template<class T>
void A65000CPU::checkPreDecrementOperator(const int &registerConfiguration)
{
    if(registerConfiguration & 0b1000) // pre-decrement?
    {
        uint8 registerSelector = mmu->read<uint8>(PC) & 0xf;
        registers[registerSelector] -= sizeof(T);
    }
}

//==============================================================================

template<class T>
void A65000CPU::checkPostIncrementOperator(const int &registerConfiguration)
{
    if(registerConfiguration & 0b100) // post-increment?
    {
        uint8 registerSelector = mmu->read<uint8>(PC) & 0xf;
        registers[registerSelector] += sizeof(T);
    }
}

//==============================================================================

template<class T>
T A65000CPU::exec_Add(const T &value1, const T &value2, const bool &withCarry)
{
    const int64 result = value1 + value2 + (withCarry ? statusRegister.c : 0);
    modifyFlagsCV(value1, value2, result);

    return (T)result;
}

//==============================================================================

template<class T>
T A65000CPU::exec_Sub(const T &value1, const T &value2, const bool &withCarry)
{
    const int64 result = value1 - value2 - (withCarry ? statusRegister.c : 0);
    modifyFlagsCV(value1, value2, result);
    
    return (T)result;
}

//==============================================================================

template<class T>
T A65000CPU::exec_Div(const T &value1, const T &value2)
{
    registers[13] = value1 % value2;
    return value1 / value2;
}

//==============================================================================

template<class T>
T A65000CPU::exec_Mul(const T &value1, const T &value2)
{
    if(value2 == 0)
        throw A65000Exception(EX_DIVISION_BY_ZERO);
    
    const uint64 result = value1 * value2;
    registers[13] = (result & 0xffff0000) >> 16;
    return result & 0xffff;
}

//==============================================================================

const uint64 shiftmask(int bits)
{
    switch(bits)
    {
        case 1:  return 0xff00;
        case 2:  return 0xffff0000;
        case 4:  return 0xffffffff00000000;
        default: return 0;
    }
}

//==============================================================================

template<class T>
T A65000CPU::exec_Rol(const T &value, T amount) const
{
    const int bitSize = sizeof(T);
    amount &= ((bitSize << 3) - 1);
    uint64 tmp = value << amount;
    uint64 result = tmp | ((tmp & shiftmask(bitSize)) >> (bitSize << 3));
    return (T)result;    
}

//==============================================================================

template<class T>
T A65000CPU::exec_Ror(const T &value, T amount) const
{
    int bitSize = sizeof(T);
    amount &= ((bitSize << 3) - 1);
    uint64 tmp = (value << (bitSize << 3)) >> amount;
    uint64 result = tmp | ((tmp & shiftmask(bitSize)) >> (bitSize << 3));
    return (T)result;
    // result = (value >> amount) | (value << (sizeof(T)*8 - amount));
}

//==============================================================================

template<class T>
T A65000CPU::executeALUInstructions(const int &instruction, const T &value1, const T &value2)
{
    switch(instruction)
    {
        case I_ADD: return exec_Add(value1, value2);
        case I_SUB: return exec_Sub(value1, value2);
        case I_ADC: return exec_Add(value1, value2, true);
        case I_SBC: return exec_Sub(value1, value2, true);
        case I_AND: return value1 & value2;
        case I_OR:  return value1 | value2;
        case I_XOR: return value1 ^ value2;
        case I_DIV: return exec_Div(value1, value2);
        case I_MUL: return exec_Mul(value1, value2);
        case I_SHL: return value1 << value2;
        case I_SHR: return value1 >> value2;
        case I_ROL: return exec_Rol(value1, value2);
        case I_ROR: return exec_Ror(value1, value2);
        default: throw A65000Exception(EX_INVALID_INSTRUCTION);
    }
}

//==============================================================================

auto A65000CPU::fetchInstructionWord() -> InstructionWord
{
    const uint16 instructionWordTmp = fetchAndAdvancePC<uint16>();

    const InstructionWord &instructionWord = *(InstructionWord *)&instructionWordTmp;

    return instructionWord;
}

//==============================================================================

uint8 A65000CPU::fetchSingleRegisterSelector()
{
    const uint8 regSelector = fetchAndAdvancePC<uint8>();
    checkRegisterRange(regSelector);
    return regSelector;  
}

//==============================================================================

auto A65000CPU::fetchRegisterPair() -> RegisterIndexPair
{
    const uint8 regSelector = fetchAndAdvancePC<uint8>();
    const int registerIndexLeft = (regSelector & 0xf0) >> 4;
    const int registerIndexRight = regSelector & 0xf;
    
    return RegisterIndexPair(registerIndexLeft, registerIndexRight);
}

//==============================================================================

template<class T>
T A65000CPU::fetchAndAdvancePC()
{
    const T value = mmu->read<T>(PC);
    PC += sizeof(T);
    return value;
}

//==============================================================================

template<typename T>
void A65000CPU::writeRegister(uint32 *reg, const T &value)
{
    *(T *)reg = value;
    modifyFlagsNZ(value);
    
    char tmp[64];
    snprintf(tmp, 64, "[%.8X] r[%d] = %X", PC, *reg, value);
//    log.push_back(tmp);
    puts(tmp);
}

//==============================================================================

void A65000CPU::checkRegisterRange(const int8 &reg) const
{
    if(reg > REG_PC || reg < REG_R0)
        throw A65000Exception(EX_INVALID_INSTRUCTION);
}

//==============================================================================

template<class T>
void A65000CPU::modifyFlagsNZ(const T &value)
{
    typedef typename std::make_signed<T>::type SignedT; // TODO: verify the conversion
    if((SignedT)value < 0) statusRegister.n = 1; else statusRegister.n = 0;
    if(value == 0) statusRegister.z = 1; else statusRegister.z = 0;
}

//==============================================================================

template<class T>
void A65000CPU::modifyFlagsCV(const T &value1, const T &value2, const int64 &result)
{
    std::numeric_limits<T> limits; // TODO: verify limits.min() and limits.max()
    if((uint64)result > (uint64)limits.max())
        statusRegister.c = 1;
    else
        statusRegister.c = 0;
    
    if(result > limits.max() || result < limits.min())
        statusRegister.v = 1;
    else
        statusRegister.v = 0;
}

//==============================================================================

#include "InstructionsMonadic.cpp"
#include "InstructionsDyadic.cpp"
