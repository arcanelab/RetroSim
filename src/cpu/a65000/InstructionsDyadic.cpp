//
//  InstructionsDyadic.cpp
//  A65000CPU
//
//  Created by Zoltán Majoros on 11/4/14.
//  Copyright (c) 2014 Zoltán Majoros. All rights reserved.
//

// Implementation of instructions with two operands

//==============================================================================

template<class T>
int A65000CPU::handleAddressingMode_IndexedDst(const InstructionWord &inst) // sub.w [$1000+r0]-, r1
{
    checkPreDecrementOperator<T>(inst.registerConfiguration);
    
    const T operandConstant = mmu->read<uint32>(PC+1);
    const T cycles = handleAddressingMode_RegisterIndirectDst<T>(inst, operandConstant);
    PC += sizeof(uint32);
    
    checkPostIncrementOperator<T>(inst.registerConfiguration);
    
    return (inst.instructionCode == I_MOV || inst.instructionCode == I_CMP) ? cycles : cycles + 1;
}

//==============================================================================

template<class T>
int A65000CPU::handleAddressingMode_IndexedSrc(const InstructionWord &inst) // DONE // add.w r0, [r1 + const]
{
    checkPreDecrementOperator<T>(inst.registerConfiguration);
    
    const T operandConstant = mmu->read<uint32>(PC+1);
    const T cycles = handleAddressingMode_RegisterIndirectSrc<T>(inst, operandConstant);
    PC += sizeof(uint32);
    
    checkPostIncrementOperator<T>(inst.registerConfiguration);
    
    return cycles + 1;
}

//==============================================================================

template<class T>
int A65000CPU::handleAddressingMode_RegisterIndirectDst(const InstructionWord &inst, uint32 offset) // DONE // add.w [r1]-, r4
{
    // fetch operand (register selector)
    const RegisterIndexPair registerPair = fetchRegisterPair();
    
    // prepare values
    const uint32 destinationAddress = registers[registerPair.leftRegIndex] + offset;
    const T valueAtAddress = mmu->read<T>(destinationAddress);
    const T valueInSourceRegister = registers[registerPair.rightRegIndex];
    T result = 0;
    int cycles = 3;
    
    // decode and execute instruction
    switch(inst.instructionCode)
    {
        case I_MOV:
            result = valueInSourceRegister;
            cycles = 2;
            break;
        case I_CMP:
            result = exec_Sub(valueAtAddress, valueInSourceRegister);
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

//==============================================================================

template<class T>
int A65000CPU::handleAddressingMode_RegisterIndirectSrc(const InstructionWord &inst, uint32 offset) // DONE // add.b r0, [r2]
{
    // fetch operand (register selector)
    const RegisterIndexPair registerPair = fetchRegisterPair();
    
    // prepare values
    const int sourceRegisterIndex = registerPair.rightRegIndex;
    const int destinationRegisterIndex = registerPair.leftRegIndex;
    
    const uint32 sourceAddress = registers[sourceRegisterIndex] + offset;
    
    const T valueAtAddress = mmu->read<T>(sourceAddress);
    const T valueInDestinationRegister = registers[destinationRegisterIndex];
    
    T result = 0;
    const int cycles = 2;
    
    // decode and execute instruction
    switch(inst.instructionCode)
    {
        case I_MOV:
            result = valueAtAddress;
            break;
        case I_CMP:
            result = exec_Sub(valueInDestinationRegister, valueAtAddress);
            modifyFlagsNZ(result);
            return cycles; // we return, because we don't want to store the result
        default:
            result = executeALUInstructions((int)inst.instructionCode, valueInDestinationRegister, valueAtAddress);
    }
    
    // store result
    writeRegister(&registers[destinationRegisterIndex], result);
    
    return cycles;
}

//==============================================================================

template<class T>
int A65000CPU::handleAddressingMode_Register2(const InstructionWord &inst) // DONE // add.b r0, r2
{
    // fetch operands (register selector, 8/16/32bit constant)
    const RegisterIndexPair registerPair = fetchRegisterPair();
    
    // prepare values
    const int cycles = 1; // I think "register2" addressing mode takes 1 cycle with all instructions
    T result = 0;
    const T leftRegisterValue = registers[registerPair.leftRegIndex];
    const T rightRegisterValue = registers[registerPair.rightRegIndex];
    
    // decode and execute instruction
    switch(inst.instructionCode)
    {
        case I_MOV:
            result = rightRegisterValue;
            break;
        case I_CMP:
            result = exec_Sub(leftRegisterValue, rightRegisterValue);
            modifyFlagsNZ(result);
            return cycles; // we return, because we don't want to store the result
        default:
            result = executeALUInstructions((int)inst.instructionCode, leftRegisterValue, rightRegisterValue);
    }
    
    // store result
    writeRegister(&registers[registerPair.leftRegIndex], result);
    
    return cycles;
}

//==============================================================================

template<class T>
int A65000CPU::handleAddressingMode_RegisterImmediate(const InstructionWord &inst) // DONE // add.b r0, 1000
{
    // fetch operands (register selector, 8/16/32bit constant)
    const uint8 registerSelector = fetchSingleRegisterSelector();
    const T opcodeConstant = fetchAndAdvancePC<T>();
    
    // prepare values
    const T valueInRegister = registers[registerSelector];
    const int cycles = 1; // I think "register immediate" addressing mode takes 1 cycle with all instructions
    T result = 0;
    
    // decode and execute instruction
    switch(inst.instructionCode)
    {
        case I_MOV:
            result = opcodeConstant;
            break;
        case I_CMP:
            result = exec_Sub(valueInRegister, opcodeConstant);
            modifyFlagsNZ(result);
            return cycles; // we return, because we don't want to store the result
        default:
            result = executeALUInstructions((int)inst.instructionCode, valueInRegister, opcodeConstant);
    }
    
    // store result
    writeRegister(&registers[registerSelector], result);
    
    return cycles;
}

//==============================================================================

template<class T>
int A65000CPU::handleAddressingMode_AbsoluteSrc(const InstructionWord &inst) // DONE // mov.b r0, [$1000]
{
    // fetch operands (register selector, 32bit address)
    const uint8 registerSelector = fetchSingleRegisterSelector();
    const uint32 address = fetchAndAdvancePC<uint32>();
    
    // prepare values
    const T valueAtAddress = mmu->read<T>(address);
    const T valueInRegister = registers[registerSelector];
    T result = 0;
    int cycles = 2;
    
    // decode and execute instruction
    switch(inst.instructionCode)
    {
        case I_MOV:
            result = valueAtAddress;
            break;
        case I_CMP:
            result = exec_Sub(valueInRegister, valueAtAddress);
            modifyFlagsNZ(result);
            return 2; // we return, because we don't want to store the result
        default:
            result = executeALUInstructions((int)inst.instructionCode, valueAtAddress, valueInRegister);
    }
    
    // store result
    writeRegister(&registers[registerSelector], result);
    
    return cycles;
}

//==============================================================================

template<class T>
int A65000CPU::handleAddressingMode_AbsoluteDst(const InstructionWord &inst) // DONE //  add.w [$1320], r9
{
    // fetch operands (register selector, 32bit address)
    const uint8 registerSelector = fetchSingleRegisterSelector();
    const uint32 address = fetchAndAdvancePC<uint32>();
    
    T result = 0;
    int cycles = 3;
    const T valueAtAddress = mmu->read<T>(address);
    const T valueInRegister = registers[registerSelector];
    
    // decode and execute instruction
    switch(inst.instructionCode)
    {
        case I_MOV:
            result = valueInRegister;
            cycles = 2;
            break;
        case I_CMP:
            result = exec_Sub(valueAtAddress, valueInRegister);
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

