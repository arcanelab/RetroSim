//
//  InstructionsMonadic.cpp
//  A65000CPU
//
//  Created by Zoltán Majoros on 11/3/14.
//  Copyright (c) 2014 Zoltán Majoros. All rights reserved.
//

// Implementation of instructions with one operand

//==============================================================================

template<class T>
int A65000CPU::handleAddressingMode_Relative(const InstructionWord &inst) // BNE $40
{
    const T diff = fetchAndAdvancePC<T>();
    const int signedDiff = (int)diff;
    
    switch(inst.instructionCode)
    {
        case I_BEQ: if(statusRegister.z)  PC += signedDiff; break;
        case I_BNE: if(!statusRegister.z) PC += signedDiff; break;
        case I_BCS: if(statusRegister.c)  PC += signedDiff; break;
        case I_BCC: if(!statusRegister.c) PC += signedDiff; break;
        case I_BMI: if(statusRegister.n)  PC += signedDiff; break;
        case I_BPL: if(!statusRegister.n) PC += signedDiff; break;
        case I_BVS: if(statusRegister.v)  PC += signedDiff; break;
        case I_BVC: if(!statusRegister.v) PC += signedDiff; break;
        case I_BLT: if(statusRegister.n != statusRegister.v) PC += signedDiff; break;
        case I_BGT: if(!statusRegister.z && (statusRegister.n == statusRegister.v)) PC += signedDiff; break;
        case I_BLE: if(statusRegister.z || (statusRegister.n != statusRegister.v))  PC += signedDiff; break;
        case I_BGE: if(statusRegister.n == statusRegister.v) PC += signedDiff; break;
        case I_BRA: PC += signedDiff; break;
        default: throw A65000Exception(EX_INVALID_INSTRUCTION);
    }
    return 1;
}

//==============================================================================

template<class T> //clr, inc, dec, jmp, jsr, push, pop
int A65000CPU::handleAddressingMode_Absolute1(const InstructionWord &inst) // inc.w [$200]
{
    const T address = fetchAndAdvancePC<T>();
    return executeMonadicInstructions_Memory<T>(address, inst.instructionCode);
}

//==============================================================================

template<class T>
int A65000CPU::handleAddressingMode_ConstImmediate(const InstructionWord &inst) // push.w $300
{
    const T value = fetchAndAdvancePC<T>();
    
    if(inst.instructionCode == I_PUSH)
    {
        SP -= sizeof(T);
        mmu->write<T>(SP, value);
    }
    else
        throw A65000Exception(EX_INVALID_INSTRUCTION);
    
    return 2;
}

//==============================================================================

int A65000CPU::handleAddressingMode_Direct(const InstructionWord &inst)
{
    const uint32 address = fetchAndAdvancePC<uint32>();
    int cycles = 0;
    
    switch(inst.instructionCode)
    {
        case I_JMP:
            PC = address;
            cycles = 1;
            break;
        case I_JSR:
            SP -= 4;
            mmu->write<uint32>(SP, PC); // PUSH PC
            PC = address;
            cycles = 3;
            break;
        default:
            throw A65000Exception(EX_INVALID_INSTRUCTION);
    }
    
    assert(cycles > 0);
    
    return cycles;
}

//==============================================================================

template<class T>
void A65000CPU::exec_IncDecMemory(const uint32 &address, const int &diff)
{
    assert((diff == 1) || (diff == -1));
    
    const T value = mmu->read<T>(address);
    const int64 result = value + diff;
    mmu->write<T>(address, (T)result);
    modifyFlagsNZ(result);
    modifyFlagsCV(value, (T)1, result);
}

//==============================================================================

template<class T>
void A65000CPU::exec_IncDecRegister(const int &registerIndex, const int &diff)
{
    assert((diff == 1) || (diff == -1));
    
    checkRegisterRange(registerIndex);
    const T value = registers[registerIndex];
    const int32 result = value + diff;
    writeRegister(&registers[registerIndex], (T)result); // this sets N & Z
    
    modifyFlagsCV(value, (T)1, result);
}

//==============================================================================

template<class T> //clr, inc, dec, jmp, jsr, push, pop
int A65000CPU::handleAddressingMode_Register1(const InstructionWord &inst) // inc.b r9
{
    const uint8 registerSelector = fetchSingleRegisterSelector();
    
    switch(inst.instructionCode)
    {
        case I_CLR:
            writeRegister(&registers[registerSelector], (T)0);
            return 1;
        case I_INC:
            exec_IncDecRegister<T>(registerSelector, 1);
            return 1;
        case I_DEC:
            exec_IncDecRegister<T>(registerSelector, -1);
            return 1;
        case I_JMP:
            PC = registers[registerSelector]; // jmp.w r4
            return 1;
        case I_JSR:
            SP -= 4;
            mmu->write<uint32>(SP, PC);
            PC = registers[registerSelector];
            return 3;
        case I_PUSH:
            SP -= sizeof(T);
            mmu->write<T>(SP, registers[registerSelector]);
            return 2;
        case I_POP:
        {
            const T value = mmu->read<T>(SP);
            writeRegister(&registers[registerSelector], value);
            modifyFlagsNZ(value);
            return 2;
        }
        case I_SXB:
            writeRegister(&registers[registerSelector], (int32)(registers[registerSelector] & 0xff)); // TODO: test
            return 1;
        case I_SXW:
            writeRegister(&registers[registerSelector], (int32)(registers[registerSelector] & 0xffff)); // TODO: test
            return 1;
        default:
            throw A65000Exception(EX_INVALID_INSTRUCTION);
    }
}

//==============================================================================

template <class T>
int A65000CPU::executeMonadicInstructions_Memory(uint32 address, int instructionCode)
{
    int cycles = 0;

    switch(instructionCode)
    {
        case I_CLR:
            mmu->write<T>(address, 0);
            cycles = 2;
            break;
        case I_INC:
            exec_IncDecMemory<T>(address, 1);
            cycles = 3;
            break;
        case I_DEC:
            exec_IncDecMemory<T>(address, -1);
            cycles = 3;
            break;
        case I_JMP:
            PC = mmu->read<T>(address);
            cycles = 2;
            break;
        case I_JSR:
            SP -= 4;
            mmu->write<uint32>(SP, PC);
            PC = mmu->read<T>(address);
            cycles = 3;
            break;
        case I_PUSH:
        {
            SP -= sizeof(T);
            const T value = mmu->read<T>(address);
            mmu->write<T>(SP, value);
            cycles = 3;
            break;
        }
        case I_POP: // pop.w [r4] // mem[r4] = mem[SP], SP+=2
        {
            const T value = mmu->read<T>(SP);
            mmu->write<T>(address, value);
            modifyFlagsNZ(value);
            SP += sizeof(T);
            cycles = 3;
            break;
        }
        case I_SXB:
            mmu->write<int32>(address, (int32)mmu->read<int8>(address)); // TODO: test
            cycles = 3;
            break;
        case I_SXW:
            mmu->write<int32>(address, (int32)mmu->read<int16>(address)); // TODO: test
            cycles = 3;
            break;
        default:
            throw A65000Exception(EX_INVALID_INSTRUCTION);
    }
    
    assert(cycles > 0);
    
    return cycles;
}

//==============================================================================

template<class T>
int A65000CPU::handleAddressingMode_Indexed1(const InstructionWord &inst) // inc.b [r0 + $300]+
{
    const int32 offset = mmu->read<uint32>(PC+1); // preload offset
    const int cycles = handleAddressingMode_RegisterIndirect<T>(inst, offset);
    PC += 4;
    return cycles + 1;
}

//==============================================================================

template<class T> //clr, inc, dec, jmp, jsr, push, pop
int A65000CPU::handleAddressingMode_RegisterIndirect(const InstructionWord &inst, uint32 offset) // inc.b [r0]+, clr.w [r2]-
{
    checkPreDecrementOperator<T>(inst.registerConfiguration);
    
    int cycles = 0;
    const uint32 address = registers[fetchSingleRegisterSelector()] + offset;
    cycles = executeMonadicInstructions_Memory<T>(address, inst.instructionCode);
    
    checkPostIncrementOperator<T>(inst.registerConfiguration);
    
    assert(cycles > 0);
    
    return cycles;
}
