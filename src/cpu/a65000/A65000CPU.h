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

#include "RSTypes.h"
#include "ICPUInterface.h"

#include <vector> // for logging
#include <string>

using std::string;
using std::vector;

class A65000MMU;

// ==============================================================================

class A65000CPU: public ICPUInterface
{
public:
    A65000CPU(A65000MMU *mmu): mmu(mmu)
    {
        reset();
    }
    // ICPUInterface methods
    int tick(); // returns the number of cycles spent
    void interruptRaised(bool isNMI = false);
    void reset();
    void setPC(unsigned int newPC);
    
    // --- enumerations ---
    
    enum Registers
    {
        REG_R0, REG_R1, REG_R2, REG_R3,
        REG_R4, REG_R5, REG_R6, REG_R7,
        REG_R8, REG_R9, REG_R10,REG_R11,
        REG_R12,REG_R13,REG_SP, REG_PC
    };

public:
    enum SystemVectors
    {
        VEC_RESET               = 0x00,
        VEC_STACKPOINTERINIT    = 0x04,
        VEC_HWIRQ               = 0x08,
        VEC_SOFTIRQ             = 0x0C,
        VEC_NMI                 = 0x10,
        VEC_ILLEGALINSTRUCTION  = 0x14,
        VEC_ILLEGALADDRESS      = 0x18,
        VEC_DIVZERO             = 0x1C,
    };

    enum Instructions
    {
        I_MOV,  I_CLR, I_ADD,   I_SUB,  I_INC,
        I_DEC,  I_MUL, I_DIV,   I_AND,  I_OR,
        I_XOR,  I_SHL, I_SHR,   I_ROL,  I_ROR,
        I_CMP,  I_SEC, I_CLC,   I_SEI,  I_CLI,
        I_PUSH, I_POP, I_PUSHA, I_POPA, I_JMP,
        I_JSR,  I_RTS, I_RTI,   I_BRK,  I_NOP,
        I_BRA,  I_BEQ, I_BNE,   I_BCC,  I_BCS,
        I_BPL,  I_BMI, I_BVC,   I_BVS,  I_BLT,
        I_BGT,  I_BLE, I_BGE,   I_SEV,  I_CLV,
        I_SLP,  I_ADC, I_SBC,   I_SXB,  I_SXW
    };

    enum AddressingModes
    {
        AM_NONE,                    //
        AM_REG_IMMEDIATE,           // Rx, 55
        AM_CONST_IMMEDIATE,         // 555 (for PUSH)
        AM_REGISTER1,               // Rx
        AM_REGISTER2,               // Rx, Ry
        AM_ABSOLUTE1,               // [$f000]
        AM_ABSOLUTE_SRC,            // Rx, [$f000]
        AM_ABSOLUTE_DEST,           // [$f000], Rx
        AM_REGISTER_INDIRECT1,      // [Rx]
        AM_REGISTER_INDIRECT_SRC,   // Rx, [Ry]
        AM_REGISTER_INDIRECT_DEST,  // [Rx], Ry
        AM_INDEXED1,                // [Rx + 234]
        AM_INDEXED_SRC,             // Rx, [Ry + 432]
        AM_INDEXED_DEST,            // [Rx + 432], Ry
        AM_RELATIVE,                // 44              -- only branching instr.
        AM_DIRECT,                  // $4244           -- only jmp/jsr
        //AM_AMBIGOUS                 // used in getAddressingModeFromOperand() when encountering OS_CONSTANT, which can be either AM_IMMEDIATE or AM_RELATIVE or AM_DIRECT
    };
    
    enum PostfixType
    {
        PF_NONE,
        PF_INC,
        PF_DEC
    };
    
    enum OpcodeSize
    {
        OS_NONE    = 0,
        OS_32BIT   = 0,
        OS_16BIT   = 1,
        OS_8BIT    = 2,
        OS_DIVSIGN = 3
    };

    // --- data structures ---
    
    struct RegisterIndexPair
    {
        int leftRegIndex;
        int rightRegIndex;
        
        RegisterIndexPair(int leftRegIndex, int rightRegIndex) : leftRegIndex(leftRegIndex), rightRegIndex(rightRegIndex){}
    };
    
    struct StatusRegister
    {
        uint8 z:1;
        uint8 n:1;
        uint8 c:1;
        uint8 v:1;
        uint8 b:1;
        uint8 i:1;
        uint8 reserved:2;
        
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
        uint16 addressingMode: 4;
        uint16 registerConfiguration: 4;
        uint16 instructionCode: 6;
        uint16 opcodeSize: 2;
    };
    
    A65000MMU *mmu; // initialized with constructor

public:
    // --- CPU state ---
    
    uint32 registers[16];
    uint32 &SP = registers[14];
    uint32 &PC = registers[15];
    
    StatusRegister statusRegister;
    
    bool sleep = false;

private:
    vector<string> log;
    
    // --- method declarations ---
    
    int runNextInstruction();
    InstructionWord fetchInstructionWord();
    uint8 fetchSingleRegisterSelector();
    RegisterIndexPair fetchRegisterPair();    
    template<class T> T fetchAndAdvancePC();
    template<class T> int decodeInstruction(const InstructionWord &inst);
    template<class T> void writeRegister(uint32 *reg, const T &value);
    template<class T> void modifyFlagsNZ(const T &value);
    template<class T> void modifyFlagsCV(const T &value1, const T &value2, const int64 &result);
    
    template<class T> int handleAddressingMode_AbsoluteSrc(const InstructionWord &inst);
    template<class T> int handleAddressingMode_AbsoluteDst(const InstructionWord &inst);
    template<class T> int handleAddressingMode_RegisterImmediate(const InstructionWord &inst);
    template<class T> int handleAddressingMode_Register2(const InstructionWord &inst);
    template<class T> int handleAddressingMode_RegisterIndirectSrc(const InstructionWord &inst, uint32 offset = 0);
    template<class T> int handleAddressingMode_RegisterIndirectDst(const InstructionWord &inst, uint32 offset = 0);
    template<class T> int handleAddressingMode_IndexedSrc(const InstructionWord &inst);
    template<class T> int handleAddressingMode_IndexedDst(const InstructionWord &inst);
    template<class T> int handleAddressingMode_RegisterIndirect(const InstructionWord &inst, uint32 offset = 0);
    template<class T> int handleAddressingMode_Register1(const InstructionWord &inst);
    template<class T> int handleAddressingMode_Indexed1(const InstructionWord &inst);
    template<class T> int handleAddressingMode_ConstImmediate(const InstructionWord &inst);
    template<class T> int handleAddressingMode_Absolute1(const InstructionWord &inst);
    template<class T> int handleAddressingMode_Relative(const InstructionWord &inst);
    template<class T> int executeMonadicInstructions_Memory(uint32 address, int instructionCode);
    int handleAddressingMode_Implied(const InstructionWord &inst);
    int handleAddressingMode_Direct(const InstructionWord &inst);
    
    template<class T> T executeALUInstructions(const int &instruction, const T &value1, const T &value2);
    template<class T> T exec_Add(const T &value1, const T &value2, const bool &withCarry = false);
    template<class T> T exec_Sub(const T &value1, const T &value2, const bool &withCarry = false);
    template<class T> T exec_And(const T &value1, const T &value2);
    template<class T> T exec_Div(const T &value1, const T &value2);
    template<class T> T exec_Mul(const T &value1, const T &value2);
    template<class T> T exec_Rol(const T &value, T amount) const;
    template<class T> T exec_Ror(const T &value, T amount) const;
    
    template<class T> void exec_IncDecMemory(const uint32 &address, const int &diff);
    template<class T> void exec_IncDecRegister(const int &registerIndex, const int &diff);
    template<class T> void checkPreDecrementOperator(const int &registerConfiguration);
    template<class T> void checkPostIncrementOperator(const int &registerConfiguration);
    void checkRegisterRange(const int8 &reg) const;
};

// ==============================================================================
