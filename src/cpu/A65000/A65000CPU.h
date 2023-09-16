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
#pragma once

#include "ICPUInterface.h"
#include "A65000Exception.h"
#include "A65000MMU.h"

#include <vector> // for logging
#include <string>

using std::string;
using std::vector;

class A65000MMU;

// ==============================================================================

class A65000CPU : public ICPUInterface
{
public:
    A65000CPU(A65000MMU *mmu) : mmu(mmu)
    {
        reset();
    }
    // ICPUInterface methods
    int tick(); // returns the number of cycles spent
    void interruptRaised(bool isNMI = false);
    void reset();
    void setPC(unsigned int newPC);

    void checkRegisterRange(const int8_t &reg) const;

    // --- enumerations ---

    enum Registers
    {
        REG_R0,
        REG_R1,
        REG_R2,
        REG_R3,
        REG_R4,
        REG_R5,
        REG_R6,
        REG_R7,
        REG_R8,
        REG_R9,
        REG_R10,
        REG_R11,
        REG_R12,
        REG_R13,
        REG_SP,
        REG_PC
    };

public:
    enum SystemVectors
    {
        VEC_RESET = 0x00,
        VEC_STACKPOINTERINIT = 0x04,
        VEC_HWIRQ = 0x08,
        VEC_SOFTIRQ = 0x0C,
        VEC_NMI = 0x10,
        VEC_ILLEGALINSTRUCTION = 0x14,
        VEC_ILLEGALADDRESS = 0x18,
        VEC_DIVZERO = 0x1C,
    };

    enum Instructions
    {
        I_MOV,
        I_CLR,
        I_ADD,
        I_SUB,
        I_INC,
        I_DEC,
        I_MUL,
        I_DIV,
        I_AND,
        I_OR,
        I_XOR,
        I_SHL,
        I_SHR,
        I_ROL,
        I_ROR,
        I_CMP,
        I_SEC,
        I_CLC,
        I_SEI,
        I_CLI,
        I_PUSH,
        I_POP,
        I_PUSHA,
        I_POPA,
        I_JMP,
        I_JSR,
        I_RTS,
        I_RTI,
        I_BRK,
        I_NOP,
        I_BRA,
        I_BEQ,
        I_BNE,
        I_BCC,
        I_BCS,
        I_BPL,
        I_BMI,
        I_BVC,
        I_BVS,
        I_BLT,
        I_BGT,
        I_BLE,
        I_BGE,
        I_SEV,
        I_CLV,
        I_SLP,
        I_ADC,
        I_SBC,
        I_SXB,
        I_SXW
    };

    enum AddressingModes
    {
        AM_NONE,                   //
        AM_REG_IMMEDIATE,          // Rx, 55
        AM_CONST_IMMEDIATE,        // 555 (for PUSH)
        AM_REGISTER1,              // Rx
        AM_REGISTER2,              // Rx, Ry
        AM_ABSOLUTE1,              // [$f000]
        AM_ABSOLUTE_SRC,           // Rx, [$f000]
        AM_ABSOLUTE_DEST,          // [$f000], Rx
        AM_REGISTER_INDIRECT1,     // [Rx]
        AM_REGISTER_INDIRECT_SRC,  // Rx, [Ry]
        AM_REGISTER_INDIRECT_DEST, // [Rx], Ry
        AM_INDEXED1,               // [Rx + 234]
        AM_INDEXED_SRC,            // Rx, [Ry + 432]
        AM_INDEXED_DEST,           // [Rx + 432], Ry
        AM_RELATIVE,               // 44              -- only branching instr.
        AM_DIRECT,                 // $4244           -- only jmp/jsr
        // AM_AMBIGOUS                 // used in getAddressingModeFromOperand() when encountering OS_CONSTANT, which can be either AM_IMMEDIATE or AM_RELATIVE or AM_DIRECT
    };

    enum PostfixType
    {
        PF_NONE,
        PF_INC,
        PF_DEC
    };

    enum OpcodeSize
    {
        OS_NONE = 0,
        OS_32BIT = 0,
        OS_16BIT = 1,
        OS_8BIT = 2,
        OS_DIVSIGN = 3
    };

    // --- data structures ---

    struct RegisterIndexPair
    {
        int leftRegIndex;
        int rightRegIndex;

        RegisterIndexPair(int leftRegIndex, int rightRegIndex) : leftRegIndex(leftRegIndex), rightRegIndex(rightRegIndex) {}
    };

    struct StatusRegister
    {
        uint8_t z : 1;
        uint8_t n : 1;
        uint8_t c : 1;
        uint8_t v : 1;
        uint8_t b : 1;
        uint8_t i : 1;
        uint8_t reserved : 2;

        StatusRegister()
        {
            z = 0;
            n = 0;
            c = 0;
            v = 0;
            b = 0;
            i = 0;
            reserved = 0;
        }
    };

    struct InstructionWord
    {
        uint16_t addressingMode : 4;
        uint16_t registerConfiguration : 4;
        uint16_t instructionCode : 6;
        uint16_t opcodeSize : 2;
    };

    A65000MMU *mmu; // initialized with constructor

public:
    // --- CPU state ---

    uint32_t registers[16];
    uint32_t &SP = registers[14];
    uint32_t &PC = registers[15];

    StatusRegister statusRegister;

    bool sleep = false;

private:
    vector<string> log;

    // --- method declarations ---

    int runNextInstruction();
    InstructionWord fetchInstructionWord();
    uint8_t fetchSingleRegisterSelector();
    RegisterIndexPair fetchRegisterPair();

    int handleAddressingMode_Implied(const InstructionWord &inst);
    int handleAddressingMode_Direct(const InstructionWord &inst);

    template <class T>
    int decodeInstruction(const InstructionWord &instr)
    {
        switch (instr.addressingMode)
        {
        case AM_REG_IMMEDIATE:
            return handleAddressingMode_RegisterImmediate<T>(instr); // sub.w r0, 20
        case AM_REGISTER2:
            return handleAddressingMode_Register2<T>(instr); // sub.w r0, r1
        case AM_ABSOLUTE_SRC:
            return handleAddressingMode_AbsoluteSrc<T>(instr); // sub.w r0, [$a000]
        case AM_ABSOLUTE_DEST:
            return handleAddressingMode_AbsoluteDst<T>(instr); // sub.w [$a000], r0
        case AM_REGISTER_INDIRECT_SRC:
            return handleAddressingMode_RegisterIndirectSrc<T>(instr, 0); // sub.w r0, [r1]
        case AM_REGISTER_INDIRECT_DEST:
            return handleAddressingMode_RegisterIndirectDst<T>(instr, 0); // sub.w [r0], r1
        case AM_INDEXED_SRC:
            return handleAddressingMode_IndexedSrc<T>(instr); // sub.w r0, [$1000+r1]
        case AM_INDEXED_DEST:
            return handleAddressingMode_IndexedDst<T>(instr); // sub.w [$1000+r0], r1
        case AM_REGISTER_INDIRECT1:
            return handleAddressingMode_RegisterIndirect<T>(instr, 0); // inc.w [r1]-
        case AM_INDEXED1:
            return handleAddressingMode_Indexed1<T>(instr); // inc.w [r1 + $200]
        case AM_REGISTER1:
            return handleAddressingMode_Register1<T>(instr); // inc.w r1
        case AM_DIRECT:
            return handleAddressingMode_Direct(instr); // jmp $100
        case AM_CONST_IMMEDIATE:
            return handleAddressingMode_ConstImmediate<T>(instr); // push.w $330
        case AM_ABSOLUTE1:
            return handleAddressingMode_Absolute1<T>(instr); // inc.w [$200]
        case AM_RELATIVE:
            return handleAddressingMode_Relative<T>(instr); // bne -50
        case AM_NONE:
            return handleAddressingMode_Implied(instr); // rts, cli, etc
        default:
            throw A65000Exception(EX_INVALID_INSTRUCTION);
        }
    }

    const uint64_t shiftmask(int bits) const
    {
        switch (bits)
        {
        case 1:
            return 0xff00;
        case 2:
            return 0xffff0000;
        case 4:
            return 0xffffffff00000000;
        default:
            return 0;
        }
    }

    template <class T>
    void modifyFlagsNZ(const T &value)
    {
        typedef typename std::make_signed<T>::type SignedT; // TODO: verify the conversion
        if ((SignedT)value < 0)
            statusRegister.n = 1;
        else
            statusRegister.n = 0;
        if (value == 0)
            statusRegister.z = 1;
        else
            statusRegister.z = 0;
    }

    template <class T>
    void modifyFlagsCV(const T &value1, const T &value2, const int64_t &result)
    {
        std::numeric_limits<T> limits; // TODO: verify limits.min() and limits.max()
        if ((uint64_t)result > (uint64_t)limits.max())
            statusRegister.c = 1;
        else
            statusRegister.c = 0;

        if (result > limits.max() || result < limits.min())
            statusRegister.v = 1;
        else
            statusRegister.v = 0;
    }

    template <class T>
    void checkPreDecrementOperator(const int &registerConfiguration)
    {
        if (registerConfiguration & 0b1000) // pre-decrement?
        {
            uint8_t registerSelector = mmu->read<uint8_t>(PC) & 0xf;
            registers[registerSelector] -= sizeof(T);
        }
    }

    template <class T>
    void checkPostIncrementOperator(const int &registerConfiguration)
    {
        if (registerConfiguration & 0b100) // post-increment?
        {
            uint8_t registerSelector = mmu->read<uint8_t>(PC) & 0xf;
            registers[registerSelector] += sizeof(T);
        }
    }

    template <class T>
    T exec_Add(const T &value1, const T &value2, const bool &withCarry)
    {
        const int64_t result = value1 + value2 + (withCarry ? statusRegister.c : 0);
        modifyFlagsCV(value1, value2, result);

        return (T)result;
    }

    template <class T>
    T exec_Sub(const T &value1, const T &value2, const bool &withCarry)
    {
        const int64_t result = value1 - value2 - (withCarry ? statusRegister.c : 0);
        modifyFlagsCV(value1, value2, result);

        return (T)result;
    }

    template <class T>
    T exec_Div(const T &value1, const T &value2)
    {
        registers[13] = value1 % value2;
        return value1 / value2;
    }

    template <class T>
    T exec_Mul(const T &value1, const T &value2)
    {
        if (value2 == 0)
            throw A65000Exception(EX_DIVISION_BY_ZERO);

        const uint64_t result = value1 * value2;
        registers[13] = (result & 0xffff0000) >> 16;
        return result & 0xffff;
    }

    template <class T>
    T exec_Rol(const T &value, T amount) const
    {
        const int bitSize = sizeof(T);
        amount &= ((bitSize << 3) - 1);
        uint64_t tmp = value << amount;
        uint64_t result = tmp | ((tmp & shiftmask(bitSize)) >> (bitSize << 3));
        return (T)result;
    }

    template <class T>
    T exec_Ror(const T &value, T amount) const
    {
        int bitSize = sizeof(T);
        amount &= ((bitSize << 3) - 1);
        uint64_t tmp = (value << (bitSize << 3)) >> amount;
        uint64_t result = tmp | ((tmp & shiftmask(bitSize)) >> (bitSize << 3));
        return (T)result;
        // result = (value >> amount) | (value << (sizeof(T)*8 - amount));
    }

    template <class T>
    T fetchAndAdvancePC()
    {
        const T value = mmu->read<T>(PC);
        PC += sizeof(T);
        return value;
    }

    template <class T>
    T executeALUInstructions(const int &instruction, const T &value1, const T &value2)
    {
        switch (instruction)
        {
        case I_ADD:
            return exec_Add(value1, value2, false);
        case I_SUB:
            return exec_Sub(value1, value2, false);
        case I_ADC:
            return exec_Add(value1, value2, true);
        case I_SBC:
            return exec_Sub(value1, value2, true);
        case I_AND:
            return value1 & value2;
        case I_OR:
            return value1 | value2;
        case I_XOR:
            return value1 ^ value2;
        case I_DIV:
            return exec_Div(value1, value2);
        case I_MUL:
            return exec_Mul(value1, value2);
        case I_SHL:
            return value1 << value2;
        case I_SHR:
            return value1 >> value2;
        case I_ROL:
            return exec_Rol(value1, value2);
        case I_ROR:
            return exec_Ror(value1, value2);
        default:
            throw A65000Exception(EX_INVALID_INSTRUCTION);
        }
    }

    template <typename T>
    void writeRegister(uint32_t *reg, const T &value)
    {
        *(T *)reg = value;
        modifyFlagsNZ(value);

        char tmp[64];
        snprintf(tmp, 64, "[%.8X] r[%d] = %X", PC, *reg, value);
        //    log.push_back(tmp);
        puts(tmp);
    }

    template <class T>
    int handleAddressingMode_Relative(const InstructionWord &inst) // BNE $40
    {
        const T diff = fetchAndAdvancePC<T>();
        const int signedDiff = (int)diff;

        switch (inst.instructionCode)
        {
        case I_BEQ:
            if (statusRegister.z)
                PC += signedDiff;
            break;
        case I_BNE:
            if (!statusRegister.z)
                PC += signedDiff;
            break;
        case I_BCS:
            if (statusRegister.c)
                PC += signedDiff;
            break;
        case I_BCC:
            if (!statusRegister.c)
                PC += signedDiff;
            break;
        case I_BMI:
            if (statusRegister.n)
                PC += signedDiff;
            break;
        case I_BPL:
            if (!statusRegister.n)
                PC += signedDiff;
            break;
        case I_BVS:
            if (statusRegister.v)
                PC += signedDiff;
            break;
        case I_BVC:
            if (!statusRegister.v)
                PC += signedDiff;
            break;
        case I_BLT:
            if (statusRegister.n != statusRegister.v)
                PC += signedDiff;
            break;
        case I_BGT:
            if (!statusRegister.z && (statusRegister.n == statusRegister.v))
                PC += signedDiff;
            break;
        case I_BLE:
            if (statusRegister.z || (statusRegister.n != statusRegister.v))
                PC += signedDiff;
            break;
        case I_BGE:
            if (statusRegister.n == statusRegister.v)
                PC += signedDiff;
            break;
        case I_BRA:
            PC += signedDiff;
            break;
        default:
            throw A65000Exception(EX_INVALID_INSTRUCTION);
        }
        return 1;
    }

    template <class T>                                              // clr, inc, dec, jmp, jsr, push, pop
    int handleAddressingMode_Absolute1(const InstructionWord &inst) // inc.w [$200]
    {
        const T address = fetchAndAdvancePC<T>();
        return executeMonadicInstructions_Memory<T>(address, inst.instructionCode);
    }

    template <class T>
    int handleAddressingMode_ConstImmediate(const InstructionWord &inst) // push.w $300
    {
        const T value = fetchAndAdvancePC<T>();

        if (inst.instructionCode == I_PUSH)
        {
            SP -= sizeof(T);
            mmu->write<T>(SP, value);
        }
        else
            throw A65000Exception(EX_INVALID_INSTRUCTION);

        return 2;
    }

    template <class T>
    void exec_IncDecMemory(const uint32_t &address, const int &diff)
    {
        assert((diff == 1) || (diff == -1));

        const T value = mmu->read<T>(address);
        const int64_t result = value + diff;
        mmu->write<T>(address, (T)result);
        modifyFlagsNZ(result);
        modifyFlagsCV(value, (T)1, result);
    }

    template <class T>
    void exec_IncDecRegister(const int &registerIndex, const int &diff)
    {
        assert((diff == 1) || (diff == -1));

        checkRegisterRange(registerIndex);
        const T value = registers[registerIndex];
        const int32_t result = value + diff;
        writeRegister(&registers[registerIndex], (T)result); // this sets N & Z

        modifyFlagsCV(value, (T)1, result);
    }

    template <class T>                                              // clr, inc, dec, jmp, jsr, push, pop
    int handleAddressingMode_Register1(const InstructionWord &inst) // inc.b r9
    {
        const uint8_t registerSelector = fetchSingleRegisterSelector();

        switch (inst.instructionCode)
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
            mmu->write<uint32_t>(SP, PC);
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
            writeRegister(&registers[registerSelector], (int32_t)(registers[registerSelector] & 0xff)); // TODO: test
            return 1;
        case I_SXW:
            writeRegister(&registers[registerSelector], (int32_t)(registers[registerSelector] & 0xffff)); // TODO: test
            return 1;
        default:
            throw A65000Exception(EX_INVALID_INSTRUCTION);
        }
    }

    template <class T>
    int executeMonadicInstructions_Memory(uint32_t address, int instructionCode)
    {
        int cycles = 0;

        switch (instructionCode)
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
            mmu->write<uint32_t>(SP, PC);
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
            mmu->write<int32_t>(address, (int32_t)mmu->read<int8_t>(address)); // TODO: test
            cycles = 3;
            break;
        case I_SXW:
            mmu->write<int32_t>(address, (int32_t)mmu->read<int16_t>(address)); // TODO: test
            cycles = 3;
            break;
        default:
            throw A65000Exception(EX_INVALID_INSTRUCTION);
        }

        assert(cycles > 0);

        return cycles;
    }

    template <class T>
    int handleAddressingMode_Indexed1(const InstructionWord &inst) // inc.b [r0 + $300]+
    {
        const int32_t offset = mmu->read<uint32_t>(PC + 1); // preload offset
        const int cycles = handleAddressingMode_RegisterIndirect<T>(inst, offset);
        PC += 4;
        return cycles + 1;
    }

    template <class T>                                                                      // clr, inc, dec, jmp, jsr, push, pop
    int handleAddressingMode_RegisterIndirect(const InstructionWord &inst, uint32_t offset) // inc.b [r0]+, clr.w [r2]-
    {
        checkPreDecrementOperator<T>(inst.registerConfiguration);

        int cycles = 0;
        const uint32_t address = registers[fetchSingleRegisterSelector()] + offset;
        cycles = executeMonadicInstructions_Memory<T>(address, inst.instructionCode);

        checkPostIncrementOperator<T>(inst.registerConfiguration);

        assert(cycles > 0);

        return cycles;
    }

    template <class T>
    int handleAddressingMode_IndexedDst(const InstructionWord &inst) // sub.w [$1000+r0]-, r1
    {
        checkPreDecrementOperator<T>(inst.registerConfiguration);

        const T operandConstant = mmu->read<uint32_t>(PC + 1);
        const T cycles = handleAddressingMode_RegisterIndirectDst<T>(inst, operandConstant);
        PC += sizeof(uint32_t);

        checkPostIncrementOperator<T>(inst.registerConfiguration);

        return (inst.instructionCode == I_MOV || inst.instructionCode == I_CMP) ? cycles : cycles + 1;
    }

    template <class T>
    int handleAddressingMode_IndexedSrc(const InstructionWord &inst) // DONE // add.w r0, [r1 + const]
    {
        checkPreDecrementOperator<T>(inst.registerConfiguration);

        const T operandConstant = mmu->read<uint32_t>(PC + 1);
        const T cycles = handleAddressingMode_RegisterIndirectSrc<T>(inst, operandConstant);
        PC += sizeof(uint32_t);

        checkPostIncrementOperator<T>(inst.registerConfiguration);

        return cycles + 1;
    }

    template <class T>
    int handleAddressingMode_RegisterIndirectDst(const InstructionWord &inst, uint32_t offset) // DONE // add.w [r1]-, r4
    {
        // fetch operand (register selector)
        const RegisterIndexPair registerPair = fetchRegisterPair();

        // prepare values
        const uint32_t destinationAddress = registers[registerPair.leftRegIndex] + offset;
        const T valueAtAddress = mmu->read<T>(destinationAddress);
        const T valueInSourceRegister = registers[registerPair.rightRegIndex];
        T result = 0;
        int cycles = 3;

        // decode and execute instruction
        switch (inst.instructionCode)
        {
        case I_MOV:
            result = valueInSourceRegister;
            cycles = 2;
            break;
        case I_CMP:
            result = exec_Sub(valueAtAddress, valueInSourceRegister, false);
            modifyFlagsNZ(result);
            return 2; // we return, because we don't want to store the result
        default:
            result = executeALUInstructions((int)inst.instructionCode, valueAtAddress, valueInSourceRegister);
        }

        // store result
        mmu->write(destinationAddress, result);
        modifyFlagsNZ(result);

        return cycles;
    }

    template <class T>
    int handleAddressingMode_RegisterIndirectSrc(const InstructionWord &inst, uint32_t offset) // DONE // add.b r0, [r2]
    {
        // fetch operand (register selector)
        const RegisterIndexPair registerPair = fetchRegisterPair();

        // prepare values
        const int sourceRegisterIndex = registerPair.rightRegIndex;
        const int destinationRegisterIndex = registerPair.leftRegIndex;

        const uint32_t sourceAddress = registers[sourceRegisterIndex] + offset;

        const T valueAtAddress = mmu->read<T>(sourceAddress);
        const T valueInDestinationRegister = registers[destinationRegisterIndex];

        T result = 0;
        const int cycles = 2;

        // decode and execute instruction
        switch (inst.instructionCode)
        {
        case I_MOV:
            result = valueAtAddress;
            break;
        case I_CMP:
            result = exec_Sub(valueInDestinationRegister, valueAtAddress, false);
            modifyFlagsNZ(result);
            return cycles; // we return, because we don't want to store the result
        default:
            result = executeALUInstructions((int)inst.instructionCode, valueInDestinationRegister, valueAtAddress);
        }

        // store result
        writeRegister(&registers[destinationRegisterIndex], result);

        return cycles;
    }

    template <class T>
    int handleAddressingMode_Register2(const InstructionWord &inst) // DONE // add.b r0, r2
    {
        // fetch operands (register selector, 8/16/32bit constant)
        const RegisterIndexPair registerPair = fetchRegisterPair();

        // prepare values
        const int cycles = 1; // I think "register2" addressing mode takes 1 cycle with all instructions
        T result = 0;
        const T leftRegisterValue = registers[registerPair.leftRegIndex];
        const T rightRegisterValue = registers[registerPair.rightRegIndex];

        // decode and execute instruction
        switch (inst.instructionCode)
        {
        case I_MOV:
            result = rightRegisterValue;
            break;
        case I_CMP:
            result = exec_Sub(leftRegisterValue, rightRegisterValue, false);
            modifyFlagsNZ(result);
            return cycles; // we return, because we don't want to store the result
        default:
            result = executeALUInstructions((int)inst.instructionCode, leftRegisterValue, rightRegisterValue);
        }

        // store result
        writeRegister(&registers[registerPair.leftRegIndex], result);

        return cycles;
    }

    template <class T>
    int handleAddressingMode_RegisterImmediate(const InstructionWord &inst) // DONE // add.b r0, 1000
    {
        // fetch operands (register selector, 8/16/32bit constant)
        const uint8_t registerSelector = fetchSingleRegisterSelector();
        const T opcodeConstant = fetchAndAdvancePC<T>();

        // prepare values
        const T valueInRegister = registers[registerSelector];
        const int cycles = 1; // I think "register immediate" addressing mode takes 1 cycle with all instructions
        T result = 0;

        // decode and execute instruction
        switch (inst.instructionCode)
        {
        case I_MOV:
            result = opcodeConstant;
            break;
        case I_CMP:
            result = exec_Sub(valueInRegister, opcodeConstant, false);
            modifyFlagsNZ(result);
            return cycles; // we return, because we don't want to store the result
        default:
            result = executeALUInstructions((int)inst.instructionCode, valueInRegister, opcodeConstant);
        }

        // store result
        writeRegister(&registers[registerSelector], result);

        return cycles;
    }

    template <class T>
    int handleAddressingMode_AbsoluteSrc(const InstructionWord &inst) // DONE // mov.b r0, [$1000]
    {
        // fetch operands (register selector, 32bit address)
        const uint8_t registerSelector = fetchSingleRegisterSelector();
        const uint32_t address = fetchAndAdvancePC<uint32_t>();

        // prepare values
        const T valueAtAddress = mmu->read<T>(address);
        const T valueInRegister = registers[registerSelector];
        T result = 0;
        int cycles = 2;

        // decode and execute instruction
        switch (inst.instructionCode)
        {
        case I_MOV:
            result = valueAtAddress;
            break;
        case I_CMP:
            result = exec_Sub(valueInRegister, valueAtAddress, false);
            modifyFlagsNZ(result);
            return 2; // we return, because we don't want to store the result
        default:
            result = executeALUInstructions((int)inst.instructionCode, valueAtAddress, valueInRegister);
        }

        // store result
        writeRegister(&registers[registerSelector], result);

        return cycles;
    }

    template <class T>
    int handleAddressingMode_AbsoluteDst(const InstructionWord &inst) // DONE //  add.w [$1320], r9
    {
        // fetch operands (register selector, 32bit address)
        const uint8_t registerSelector = fetchSingleRegisterSelector();
        const uint32_t address = fetchAndAdvancePC<uint32_t>();

        T result = 0;
        int cycles = 3;
        const T valueAtAddress = mmu->read<T>(address);
        const T valueInRegister = registers[registerSelector];

        // decode and execute instruction
        switch (inst.instructionCode)
        {
        case I_MOV:
            result = valueInRegister;
            cycles = 2;
            break;
        case I_CMP:
            result = exec_Sub(valueAtAddress, valueInRegister, false);
            modifyFlagsNZ(result);
            return 2; // we return, because we don't want to store the result
        default:
            result = executeALUInstructions((int)inst.instructionCode, valueAtAddress, valueInRegister);
        }

        // store result
        mmu->write(address, result);
        modifyFlagsNZ(result);

        return cycles;
    }
};
