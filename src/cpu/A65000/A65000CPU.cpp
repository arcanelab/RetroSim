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

void A65000CPU::interruptRaised(bool isNMI)
{
    sleep = false;

    if (!isNMI && statusRegister.i)
        return;

    SP--; // save the status register to the stack
    mmu->write<uint8_t>(SP, *(uint8_t *)(&statusRegister));

    SP -= 4; // save the PC to the stack
    mmu->write<uint32_t>(SP, PC);

    if (isNMI)
        PC = mmu->read<uint32_t>(VEC_NMI); // jump to the NMI-handler
    else
        PC = mmu->read<uint32_t>(VEC_HWIRQ); // jump to the IRQ-handler
}

void A65000CPU::reset()
{
    sleep = false;

    for (int i = 0; i < 14; i++)
        registers[i] = 0;

    PC = mmu->read<uint32_t>(VEC_RESET);
    SP = mmu->read<uint32_t>(VEC_STACKPOINTERINIT);
}

void A65000CPU::setPC(unsigned int newPC)
{
    PC = (uint32_t)newPC;
}

int A65000CPU::tick() // return value: the number of cycles used
{
    if (sleep)
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
            PC = mmu->read<uint32_t>(VEC_ILLEGALINSTRUCTION);
            break;
        case EX_INVALID_ADDRESS:
            PC = mmu->read<uint32_t>(VEC_ILLEGALADDRESS);
            break;
        case EX_DIVISION_BY_ZERO:
            PC = mmu->read<uint32_t>(VEC_DIVZERO);
            break;
        default:
            // TODO: here?
            break;
        }
    }

    return 2;
}

int A65000CPU::runNextInstruction()
{
    const InstructionWord &instr = fetchInstructionWord();

    switch (instr.opcodeSize)
    {
    case OS_8BIT:
        return decodeInstruction<uint8_t>(instr);
    case OS_16BIT:
        return decodeInstruction<uint16_t>(instr);
    case OS_32BIT:
        return decodeInstruction<uint32_t>(instr);
    default:
        throw A65000Exception(EX_INVALID_INSTRUCTION);
    }
}

int A65000CPU::handleAddressingMode_Implied(const InstructionWord &inst)
{
    int cycles = 1;

    switch (inst.instructionCode)
    {
    case I_NOP:
        break;
    case I_SEI:
        statusRegister.i = 1;
        break;
    case I_CLI:
        statusRegister.i = 0;
        break;
    case I_SEC:
        statusRegister.c = 1;
        break;
    case I_CLC:
        statusRegister.c = 0;
        break;
    case I_SEV:
        statusRegister.v = 1;
        break;
    case I_CLV:
        statusRegister.v = 0;
        break;

    case I_RTS:
        PC = mmu->read<uint32_t>(SP); // POP PC
        SP += 4;
        cycles = 3;
        break;

    case I_PUSHA:
        for (int i = 0; i < 16; i++)
        {
            SP -= 4;
            mmu->write<uint32_t>(SP, registers[i]);
        }
        cycles = 32;
        break;

    case I_POPA:
        for (int i = 0; i < 16; i++)
        {
            registers[i] = mmu->read<uint32_t>(SP);
            SP += 4;
        }
        cycles = 32;
        break;

    case I_BRK:
        if (statusRegister.i)
            return 1;

        SP--;
        mmu->write<uint8_t>(SP, *(uint8_t *)(&statusRegister)); // PUSH Status

        SP -= 4;
        mmu->write<uint32_t>(SP, PC); // PUSH PC

        PC = mmu->read<uint32_t>(VEC_SOFTIRQ); // JMP [VEC_SOFTIRQ]

        statusRegister.b = 1;

        cycles = 6;
        break;

    case I_RTI:
        PC = mmu->read<uint32_t>(SP); // POP PC
        SP += 4;
        *(uint8_t *)(&statusRegister) = (uint8_t)mmu->read<uint8_t>(SP); // POP Status
        SP++;
        cycles = 5;
        break;

    case I_SLP:
        sleep = true;
        break;

    default:
        throw A65000Exception(EX_INVALID_INSTRUCTION);
    }

    return cycles;
}

auto A65000CPU::fetchInstructionWord() -> InstructionWord
{
    const uint16_t instructionWordTmp = fetchAndAdvancePC<uint16_t>();

    const InstructionWord &instructionWord = *(InstructionWord *)&instructionWordTmp;

    return instructionWord;
}

uint8_t A65000CPU::fetchSingleRegisterSelector()
{
    const uint8_t regSelector = fetchAndAdvancePC<uint8_t>();
    checkRegisterRange(regSelector);
    return regSelector;
}

auto A65000CPU::fetchRegisterPair() -> RegisterIndexPair
{
    const uint8_t regSelector = fetchAndAdvancePC<uint8_t>();
    const int registerIndexLeft = (regSelector & 0xf0) >> 4;
    const int registerIndexRight = regSelector & 0xf;

    return RegisterIndexPair(registerIndexLeft, registerIndexRight);
}

void A65000CPU::checkRegisterRange(const int8_t &reg) const
{
    if (reg > REG_PC || reg < REG_R0)
        throw A65000Exception(EX_INVALID_INSTRUCTION);
}

int A65000CPU::handleAddressingMode_Direct(const InstructionWord &inst)
{
    const uint32_t address = fetchAndAdvancePC<uint32_t>();
    int cycles = 0;

    switch (inst.instructionCode)
    {
    case I_JMP:
        PC = address;
        cycles = 1;
        break;
    case I_JSR:
        SP -= 4;
        mmu->write<uint32_t>(SP, PC); // PUSH PC
        PC = address;
        cycles = 3;
        break;
    default:
        throw A65000Exception(EX_INVALID_INSTRUCTION);
    }

    assert(cycles > 0);

    return cycles;
}
