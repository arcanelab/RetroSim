//
//  A65000Disassembler.hpp
//  A65000 Disassembler
//
//  Created by Zoltán Majoros on 26/Nov/2015.
//  Copyright © 2015 Zoltán Majoros. All rights reserved.
//

#pragma once

#include "A65000CPU.h"
#include "RSTypes.h"
#include <string>
#include <vector>

using std::string;
using std::vector;

struct A65000Disassembler
{
    struct Disassembly
    {
        vector<string> text;
        uint8 bytesProcessed;
    };
    
    A65000Disassembler();
    ~A65000Disassembler();
    
    Disassembly result;
    
    bool showMachineCode = true;
    
    vector<string> instructionNames = {"mov", "clr", "add", "sub", "inc", "dec", "mul", "div",
        "and", "or", "xor", "shl", "shr", "rol", "ror", "cmp",
        "sec", "clc", "sei", "cli", "push", "pop", "pusha",
        "popa", "jmp", "jsr", "rts", "rti", "brk", "nop", "bra",
        "beq", "bne", "bcc", "bcs", "bpl", "bmi", "bvc", "bvs",
        "blt", "bgt", "ble", "bge", "sev", "clv", "slp", "adc",
        "sbc", "sxb", "sxw"};
    
    Disassembly getDisassembly(uint8 * const codePtr, const uint32 address, const uint16 lines);    
    vector<string> getDisassembly(const char *fileName);
    
    struct Error
    {
        string errorString;
        
        Error(const vector<string> &errorStrings)
        {
            for(string s : errorStrings)
                errorString += s;
        }
        
        Error(const string &errorString)
        {
            this->errorString = errorString;
        }
    };
    
private:
    struct Chunk
    {
        uint32 address;
        uint32 length;
        uint8 *data;
        int8 maxLines = -1;
    };

    void print(string text);
    string addressStr(uint32 address);
    Disassembly disassembleChunk(Chunk chunk);
    string opcodeToString(const A65000CPU::InstructionWord &iw);
    uint8 operandSizeInBytes(const A65000CPU::OpcodeSize &size);
    string operandToString(const uint32 &operand, const A65000CPU::OpcodeSize &size);
    string addressToString(const uint32 address);
    string registerStr(const uint8 &reg);
    string machineCode(const uint8 *ptr, const int &length);
    vector<Chunk> parseRSXFileIntoChunks(vector<uint8> * data);
    vector<uint8> *loadFile(const char *loadPath);
};
