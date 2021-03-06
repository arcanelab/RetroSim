//
//  A65000Disassembler.cpp
//  A65000 Disassembler
//
//  Created by Zoltán Majoros on 10/May/2015.
//  Copyright © 2016 Zoltán Majoros. All rights reserved.
//


#include "A65000Disassembler.h"

A65000Disassembler::A65000Disassembler()
{
    
}

A65000Disassembler::~A65000Disassembler()
{
    
}

A65000Disassembler::Disassembly A65000Disassembler::getDisassembly(uint8 * const codePtr, const uint32 address, const uint16 lines = 0)
{
    Chunk chunk;
    chunk.address = address;
    chunk.data = codePtr;
    chunk.length = lines * 7;  // 7 = max. instruction length
    chunk.maxLines = lines;

    return disassembleChunk(chunk);
}

vector<string> A65000Disassembler::getDisassembly(const char *fileName)
{
    try
    {
        auto chunks = parseRSXFileIntoChunks(loadFile(fileName));
        result = disassembleChunk(chunks[0]);
    }
    catch (Error e)
    {
        puts(e.errorString.c_str());
    }

    //        return move(disassembly);
    return result.text;
}

void A65000Disassembler::print(string text)
{
    transform(text.begin(), text.end(), text.begin(), ::tolower);
    //puts(text.c_str());
    result.text.push_back(text);
}

string A65000Disassembler::addressStr(uint32 address)
{
    char textBuffer[64];
    snprintf(textBuffer, 64, "%.8X", address);
    return string(textBuffer);
}

A65000Disassembler::Disassembly A65000Disassembler::disassembleChunk(Chunk chunk)
{
    uint32 pc = chunk.address;
    if(chunk.maxLines == -1)
        chunk.maxLines = 255;

    while(chunk.maxLines--)
    {
        if(pc >= chunk.address + chunk.length)
            break;

        A65000CPU::InstructionWord iw = *(A65000CPU::InstructionWord *)&chunk.data[pc-chunk.address];
        uint8 *machineCodePtr = (uint8 *)&chunk.data[pc-chunk.address];
        uint8 *operands = &chunk.data[(pc+2) - chunk.address];
        /*
         if(*machineCodePtr == 0)
         {
         print(addressStr(pc) + machineCode(machineCodePtr, 1) + "???");
         pc++;
         }
         else
         */
        switch(iw.addressingMode)
        {
            case A65000CPU::AddressingModes::AM_NONE:
                print(addressStr(pc) + machineCode(machineCodePtr, 2) + opcodeToString(iw));
                pc += 2;
                break;
            case A65000CPU::AddressingModes::AM_REGISTER1: // clr r0
                print(addressStr(pc) + machineCode(machineCodePtr, 3) + opcodeToString(iw) + registerStr(*operands));
                pc += 3;
                break;
            case A65000CPU::AddressingModes::AM_REGISTER_INDIRECT1: // clr [r0]
                print(addressStr(pc) + machineCode(machineCodePtr, 3) + opcodeToString(iw) + "[" + registerStr(*operands) + "]");
                pc += 3;
                break;
            case A65000CPU::AddressingModes::AM_REGISTER2: // mov.b r0, r1
                print(addressStr(pc) + machineCode(machineCodePtr, 3) + opcodeToString(iw) + registerStr((*operands & 0xf0) >> 4) + ", " + registerStr(*operands & 0xf));
                pc += 3;
                break;
            case A65000CPU::AddressingModes::AM_REGISTER_INDIRECT_SRC: // mov.b r0, [r1]
                print(addressStr(pc) + machineCode(machineCodePtr, 3) + opcodeToString(iw) + registerStr((*operands & 0xf0) >> 4) + ", [" + registerStr(*operands & 0xf) + "]");
                pc += 3;
                break;
            case A65000CPU::AddressingModes::AM_REGISTER_INDIRECT_DEST: // mov.b [r0], r1
                print(addressStr(pc) + machineCode(machineCodePtr, 3) + opcodeToString(iw) + "[" + registerStr((*operands & 0xf0) >> 4) + "], " + registerStr(*operands & 0xf));
                pc += 3;
                break;
            case A65000CPU::AddressingModes::AM_REG_IMMEDIATE: // mov r0, $feffccfe
            {
                uint8 registerSelector = chunk.data[(pc+2) - chunk.address];
                uint32 operand32 = *(uint32*)&(chunk.data[(pc+3) - chunk.address]);
                string output;

                output += addressStr(pc); // $00001000
                output += machineCode(machineCodePtr, operandSizeInBytes((A65000CPU::OpcodeSize)iw.opcodeSize) + 3); // 13 01 00 11 22 33 44
                output += opcodeToString(iw);
                output += registerStr(registerSelector) + ", ";
                output += operandToString(operand32, (A65000CPU::OpcodeSize)iw.opcodeSize);
                print(output);
                pc += operandSizeInBytes((A65000CPU::OpcodeSize)iw.opcodeSize) + 3;
                break;
            }
            case A65000CPU::AddressingModes::AM_ABSOLUTE1:
            {
                uint32 operand32 = *(uint32*)&(chunk.data[(pc+2) - chunk.address]);
                string output;

                output += addressStr(pc); // $00001000
                output += machineCode(machineCodePtr, 6);
                output += opcodeToString(iw);
                output += "[" + operandToString(operand32, (A65000CPU::OpcodeSize)iw.opcodeSize) + "]";
                print(output);
                pc += 6;
                break;
            }
            case A65000CPU::AddressingModes::AM_ABSOLUTE_SRC: // Rx, [$f000]
            {
                uint8 registerSelector = chunk.data[(pc+2) - chunk.address];
                uint32 address = *(uint32*)&(chunk.data[(pc+3) - chunk.address]);
                string output;

                output += addressStr(pc);
                output += machineCode(machineCodePtr, 7);
                output += opcodeToString(iw);
                output += registerStr(registerSelector) + ", ";
                output += "[" + addressToString(address) += "]";
                print(output);
                pc += 7;
                break;
            }
            case A65000CPU::AddressingModes::AM_ABSOLUTE_DEST: // [$f000], Rx
            {
                uint8 registerSelector = chunk.data[(pc+2) - chunk.address];
                uint32 address = *(uint32*)&(chunk.data[(pc+3) - chunk.address]);
                string output;

                output += addressStr(pc);
                output += machineCode(machineCodePtr, 7);
                output += opcodeToString(iw);
                output += "[" + addressToString(address) += "], ";
                output += registerStr(registerSelector);
                print(output);
                pc += 7;
                break;
            }
            case A65000CPU::AddressingModes::AM_DIRECT: // jsr $fffeffcd
            case A65000CPU::AddressingModes::AM_CONST_IMMEDIATE: // push $fffeffcd
            {
                string output;
                uint32 operand = *(uint32*)&(chunk.data[(pc+2) - chunk.address]);

                output += addressStr(pc);
                output += machineCode(machineCodePtr, operandSizeInBytes((A65000CPU::OpcodeSize)iw.opcodeSize) + 2);
                output += opcodeToString(iw);
                output += operandToString(operand, (A65000CPU::OpcodeSize)iw.opcodeSize);
                print(output);
                pc += operandSizeInBytes((A65000CPU::OpcodeSize)iw.opcodeSize) + 2;
                break;
            }
            case A65000CPU::AddressingModes::AM_INDEXED1: // inc [r0 + $345]
            {
                uint8 registerSelector = chunk.data[(pc+2) - chunk.address];
                uint32 operand32 = *(uint32*)&(chunk.data[(pc+3) - chunk.address]);
                string output;

                output += addressStr(pc); // $00001000
                output += machineCode(machineCodePtr, 7);
                output += opcodeToString(iw);
                output += "[" + registerStr(registerSelector) + " + ";
                output += operandToString(operand32, (A65000CPU::OpcodeSize)iw.opcodeSize) + "]";
                print(output);
                pc += 7;
                break;
            }
            case A65000CPU::AddressingModes::AM_INDEXED_SRC: // mov r0, [r0 + $345]
            {
                uint8 registerSelector = chunk.data[(pc+2) - chunk.address];
                uint32 operand32 = *(uint32*)&(chunk.data[(pc+3) - chunk.address]);
                string output;

                output += addressStr(pc); // $00001000
                output += machineCode(machineCodePtr, 7);
                output += opcodeToString(iw);
                output += registerStr((registerSelector & 0xf0) >> 4) + ", ";
                output += "[" + registerStr(registerSelector & 15) + " + ";
                output += operandToString(operand32, (A65000CPU::OpcodeSize)iw.opcodeSize) + "]";
                print(output);
                pc += 7;
                break;
            }
            case A65000CPU::AddressingModes::AM_INDEXED_DEST: // mov [r1 + $345], pc
            {
                uint8 registerSelector = chunk.data[(pc+2) - chunk.address];
                uint32 operand32 = *(uint32*)&(chunk.data[(pc+3) - chunk.address]);
                string output;

                output += addressStr(pc); // $00001000
                output += machineCode(machineCodePtr, 7);
                output += opcodeToString(iw);
                output += "[" + registerStr((registerSelector & 0xf0) >> 4) + " + ";
                output += operandToString(operand32, (A65000CPU::OpcodeSize)iw.opcodeSize) + "], ";
                output += registerStr(registerSelector & 15);
                print(output);
                pc += 7;
                break;
            }
            case A65000CPU::AddressingModes::AM_RELATIVE:
            {
                int32 operand32 = *(uint32*)&(chunk.data[(pc+2) - chunk.address]);
                uint32 address = (int)pc + (int)operand32;
                string output;

                output += addressStr(pc); // $00001000
                output += machineCode(machineCodePtr, 6);
                output += opcodeToString(iw);
                output += operandToString(address, (A65000CPU::OpcodeSize)iw.opcodeSize);
                print(output);
                pc += 6;
                break;
            }

            default:
                print(addressStr(pc) + machineCode(machineCodePtr, 1) + "???");
                pc += 1;
        }
    }
    result.bytesProcessed = pc - chunk.address;
    return result;
}

string A65000Disassembler::opcodeToString(const A65000CPU::InstructionWord &iw)
{
    string output;
    if(iw.instructionCode >= instructionNames.size())
        output = "???";
    else
        output = instructionNames[iw.instructionCode];

    string space;
    int diff = 5 - (int)output.length();
    for(int i=0; i < diff; i++)
        space += " ";

    if(iw.opcodeSize == A65000CPU::OpcodeSize::OS_8BIT)
        return output + ".b" + space;
    if(iw.opcodeSize == A65000CPU::OpcodeSize::OS_16BIT)
        return output + ".w" + space;

    return output + "  " + space;
}

uint8 A65000Disassembler::operandSizeInBytes(const A65000CPU::OpcodeSize &size)
{
    switch (size)
    {
        case A65000CPU::OpcodeSize::OS_32BIT:
            return 4;
        case A65000CPU::OpcodeSize::OS_16BIT:
            return 2;
        case A65000CPU::OpcodeSize::OS_8BIT:
            return 1;
        default:
            throw; // internal error: invalid opcode size, TODO: handle this is a graceful way
    }
}

string A65000Disassembler::operandToString(const uint32 &operand, const A65000CPU::OpcodeSize &size)
{
    union
    {
        uint32 value32;
        uint16 value16;
        uint8  value8;
    } opUnion;

    opUnion.value32 = operand;

    char textBuffer[16];

    switch (size)
    {
        case A65000CPU::OpcodeSize::OS_32BIT:
            snprintf(textBuffer, 16, "$%X", opUnion.value32);
            return textBuffer;
        case A65000CPU::OpcodeSize::OS_16BIT:
            snprintf(textBuffer, 16, "$%X", opUnion.value16);
            return textBuffer;
        case A65000CPU::OpcodeSize::OS_8BIT:
            snprintf(textBuffer, 16, "$%X", opUnion.value8);
            return textBuffer;
        default:
            return "?? ?? ??";
    }
}

// ----------------------------------------------------------------------------

string A65000Disassembler::addressToString(const uint32 address)
{
    char textBuffer[16];

    snprintf(textBuffer, 16, "$%X", address);
    return textBuffer;
}

// ----------------------------------------------------------------------------

string A65000Disassembler::registerStr(const uint8 &reg)
{
    if(reg > A65000CPU::REG_PC) // TODO: shall we really throw on invalid instruction encoding?
        return "r??";
    //            throw;

    if(reg == A65000CPU::REG_PC)
        return "pc";
    if(reg == A65000CPU::REG_SP)
        return "sp";

    char textBuffer[16];
    snprintf(textBuffer, 16, "R%d", reg);
    return textBuffer;
}

// ----------------------------------------------------------------------------

string A65000Disassembler::machineCode(const uint8 *ptr, const int &length)
{
    if(showMachineCode)
    {
        string output = "   ";
        char textBuffer[64];
        for(int i=0; i<length; i++)
        {
            snprintf(textBuffer, 64, "%.2X ", *(ptr+i));
            output += textBuffer;
        }

        for(int i=0; i<7-length; i++)
        {
            output += "   ";
        }

        return output + "  ";
    }
    else
        return "   ";
}

// ----------------------------------------------------------------------------

vector<A65000Disassembler::Chunk> A65000Disassembler::parseRSXFileIntoChunks(vector<uint8> * data)
{
    vector<Chunk> chunks;

    uint32 bootAddress = 0;
    uint32 i = 4; // skip header
    int chunkNumber = 0;
    while(1)
    {
        if(i >= (data->size()-1)) break;

        Chunk c;
        c.address = *(uint32 *)&data->data()[i];;
        i += 4;
        c.length = *(uint32 *)&data->data()[i];
        i += 4;

        c.data = new uint8[c.length];
        memcpy(c.data, &data->data()[i], c.length);

        if(chunkNumber == 0)
            bootAddress = c.address;

        printf("Loaded $%X (%d) bytes into [$%X-$%X]\n", c.length, c.length, c.address, c.address+c.length-1);

        i += c.length;
        chunkNumber++;

        chunks.push_back(c);
    }

    return chunks;
}

// ----------------------------------------------------------------------------

vector<uint8> *A65000Disassembler::loadFile(const char *loadPath)
{
    FILE *file = fopen(loadPath, "r");
    if (file == nullptr)
    {
        throw Error("Error while opening file '" + string(loadPath) + string("'"));
    }

    // get filesize
    fseek(file, 0, SEEK_END);
    uint64 fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    printf("Filesize: $%llX (%llu) bytes\n", fileSize, fileSize);

    // allocate memory for file
    uint8 *buffer = new uint8[fileSize];

    printf("Loading '%s'\n", loadPath);

    // read file contents
    uint32 freadResult = (uint32)fread(buffer, 1, fileSize, file);
    if(freadResult != fileSize)
    {
        fclose(file);
        delete [] buffer;
        throw Error("Error while reading file '" + string(loadPath) + string("'"));
    }

    fclose(file);

    if(fileSize > 3) // header check
    {
        string header;
        header.push_back((char)*buffer);
        header.push_back((char)*(buffer+1));
        header.push_back((char)*(buffer+2));
        header.push_back((char)*(buffer+3));
        if(header != "RSX0")
        {
            delete [] buffer;
            throw Error("Invalid file format: RSX format expected");
        }
    }
    else // filesize too small
    {
        delete [] buffer;
        throw Error("Invalid file format: RSX format expected");
    }

    vector<uint8> *fileContent = new vector<uint8>(buffer, buffer+fileSize);
    delete [] buffer;

    return fileContent;
}
