//
//  A65000Disassembler.hpp
//  A65000 Disassembler
//
//  Created by Zoltán Majoros on 26/Nov/2015.
//  Copyright © 2015 Zoltán Majoros. All rights reserved.
//

#pragma once

#include "A65000CPU.h"
#include <string>
#include <vector>

using std::string;
using std::vector;

struct A65000Disassembler
{
    struct Disassembly
    {
        vector<string> text;
        uint8_t bytesProcessed;
    };

    A65000Disassembler() = default;
    ~A65000Disassembler() = default;

    Disassembly result;

    bool showMachineCode = true;

    std::vector<std::string> instructionNames = {
        "brk", "mov", "clr", "add", "sub", "adc", "sbc", "inc", "dec", "mul", "div",
        "and", "or", "xor", "shl", "shr", "rol", "ror", "cmp", "sec", "clc",
        "sei", "cli", "push", "pop", "pusha", "popa", "jmp", "jsr", "rts", "rti",
        "nop", "bra", "beq", "bne", "bcc", "bcs", "bpl", "bmi", "bvc",
        "bvs", "blt", "bgt", "ble", "bge", "sev", "clv", "slp", "sxb", "sxw", "sys"};

    Disassembly getDisassembly(uint8_t *const codePtr, const uint32_t address, const uint8_t lines);
    vector<string> getDisassembly(const char *fileName);

    struct Error
    {
        string errorString;

        Error(const vector<string> &errorStrings)
        {
            for (string s : errorStrings)
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
        uint32_t address;
        uint32_t length;
        uint8_t *data;
        int8_t maxLines = -1;
    };

    int maxInstructionLength = 11;

    void print(string text);
    string addressStr(uint32_t address);
    Disassembly disassembleChunk(Chunk chunk);
    string opcodeToString(const A65000CPU::InstructionWord &iw, bool omitSizeSpecifier = false);
    uint8_t operandSizeInBytes(const A65000CPU::OpcodeSize &size);
    string operandToString(const uint32_t &operand, const A65000CPU::OpcodeSize &size);
    string addressToString(const uint32_t address);
    string registerStr(const uint8_t &reg);
    string machineCode(const uint8_t *ptr, const int &length);
    vector<Chunk> parseRSXFileIntoChunks(vector<uint8_t> *data);
    vector<uint8_t> *loadFile(const char *loadPath);
};
