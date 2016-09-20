//
//  RSMonitorInterpreter.cpp
//  RetroSim
//
//  Created by Zoltán Majoros on 17/May/2015.
//  Copyright © 2016 Zoltán Majoros. All rights reserved.
//

#include "RSMonitorInterpreter.h"
#include "RSCore.h"
#include "GPU-G1.h"
#include "A65000Disassembler.h"
#include "nfd.h"
#include <vector>
#include <string>
#include <sstream>
#include <regex>
#include <codecvt>

using std::string;
using std::vector;
using std::regex;
using std::smatch;

RSMonitorInterpreter::RSMonitorInterpreter()
{
    // this->core is not initialized yet
}

void RSMonitorInterpreter::setCore(RSCore *core)
{
    this->core = core;
    a65kcpu = (A65000CPU *)core->cpu;
}

vector<string> RSMonitorInterpreter::processLine(string inputLine)
{
    if(inputLine != ".")
        lastInput = inputLine;
    
//    this->core = core;
    output.clear();
    
    if(core == nullptr)
    {
        print("Internal error: no valid pointer to core");
        return output;
    }
    
    std::transform(inputLine.begin(), inputLine.end(), inputLine.begin(), ::tolower);
    vector<string> tokens = split(inputLine.c_str());
    MonitorCommand command = recognizeCommand(tokens[0]);
    string commandStr = tokens[0];
    tokens.erase(tokens.begin()); // remove command from tokens, leaving only the arguments behind
    processCommand(command, tokens, commandStr);
    
    return output;
}

void RSMonitorInterpreter::processCommand(const MonitorCommand cmd, vector<string> arguments, const string &commandStr)
{
    switch(cmd)
    {
        case MonitorCommand::RegisterDump:
            displayRegisters();
            break;
        case MonitorCommand::DisplayGPURegisters:
            displayGPURegisters();
            break;
        case MonitorCommand::DisplayGPUSurfaceRegisters:
            displayGPUSurfaceRegisters(arguments);
            break;
        case MonitorCommand::DisplayMemoryByte:
            displayMemory(arguments, Base::byte);
            break;
        case MonitorCommand::DisplayMemoryWord:
            displayMemory(arguments, Base::word);
            break;
        case MonitorCommand::DisplayMemoryDWord:
            displayMemory(arguments, Base::dword);
            break;
        case MonitorCommand::DisplayASCII:
            if(commandStr == "ii")
            {
                arguments.push_back("showInvisibles");
            }
            displayAscii(arguments);
            break;
        case MonitorCommand::Disassemble:
            displayDisassembly(arguments);
            break;
        case MonitorCommand::Run:
            runCode(arguments);
            break;
        case MonitorCommand::Quit:
            exit(0);
            break;
        case MonitorCommand::Breakpoint:
            break;
        case MonitorCommand::SetValue:
        {
            if(commandStr=="set")
            {
                displaySetValueHelp();
                break;
            }
            if(commandStr=="setb" || commandStr=="setbyte")
            {
                setValue<uint8>(arguments);
                break;
            }
            if(commandStr=="setw" || commandStr=="setword")
            {
                setValue<uint16>(arguments);
                break;
            }
            if(commandStr=="setd" || commandStr=="setdword")
                setValue<uint32>(arguments);
        }
            break;
        case MonitorCommand::Invalid:
            break;
        case MonitorCommand::RepeatLastCommand:
            processLine(lastInput);
            break;
        case MonitorCommand::DisplayHelp:
            displayHelp(arguments);
            break;
        case MonitorCommand::ClearScreen:
            clearScreen();
            break;
        case MonitorCommand::DisplaySystemConfig:
            displaySystemConfig();
            break;
        case MonitorCommand::ChangeNumericModeToDec:
            changeNumericModeToDec();
            break;
        case MonitorCommand::ChangeNumericModeToHex:
            changeNumericModeToHex();
            break;
        case MonitorCommand::LoadRSX:
            loadRSXFile(arguments);
            break;
        case MonitorCommand::LoadBinary:
            loadBinaryFile(arguments);
            break;
        case MonitorCommand::SaveBinary:
            saveBinaryFile(arguments);
            break;
        case MonitorCommand::Step:
            step();
            break;
    }
}

// ----------------------------------------------------------------------------

void RSMonitorInterpreter::step()
{
    
}

auto RSMonitorInterpreter::recognizeCommand(const string &command) -> MonitorCommand
{
    if(command == "r"  || command == "regs")       return MonitorCommand::RegisterDump;
    if(command == "rg" || command == "gpuregs")    return MonitorCommand::DisplayGPURegisters;
    if(command == "rgs"|| command == "surfaceregs")return MonitorCommand::DisplayGPUSurfaceRegisters;
    if(command == "m"  || command == "mem")        return MonitorCommand::DisplayMemoryByte;
    if(command == "mw" || command == "memword")    return MonitorCommand::DisplayMemoryWord;
    if(command == "md" || command == "memdword")   return MonitorCommand::DisplayMemoryDWord;
    if(command == "d"  || command == "disasm")     return MonitorCommand::Disassemble;
    if(command == "g"  || command == "go")         return MonitorCommand::Run;
    if(command == "q"  || command == "quit")       return MonitorCommand::Quit;
    if(command == "b"  || command == "breakpoint") return MonitorCommand::Breakpoint;
    if(command == "l"  || command == "load")       return MonitorCommand::LoadRSX;
    if(command == "lb" || command == "loadbinary") return MonitorCommand::LoadBinary;
    if(command == "s"  || command == "save")       return MonitorCommand::SaveBinary;
    if(command == "sys"|| command == "system")     return MonitorCommand::DisplaySystemConfig;
    if(command == "cls")                           return MonitorCommand::ClearScreen;
    if(command == "dec")                           return MonitorCommand::ChangeNumericModeToDec;
    if(command == "hex")                           return MonitorCommand::ChangeNumericModeToHex;
    if(command == "i"  || command == "ascii" || command == "ii") return MonitorCommand::DisplayASCII;
    if(command == "set"||command == "setb"|| command == "setw"  || command == "setd" || command == "setbyte"|| command == "setword"  || command == "setdword") return MonitorCommand::SetValue;
    if(command == "." )   return MonitorCommand::RepeatLastCommand;
    if(command == "help") return MonitorCommand::DisplayHelp;
    if(command == "n"  || command == "step")       return MonitorCommand::Step;
    
    return MonitorCommand::Invalid;
}

// ----------------------------------------------------------------------------

void RSMonitorInterpreter::print(const string &text)
{
    output.push_back(text);
}

// ----------------------------------------------------------------------------

const std::string copyrightMessage = "Copyright (c) 2011-2016 Zoltán Majoros (zoltan@arcanelab.com)";

void RSMonitorInterpreter::displaySystemConfig()
{
    print(copyrightMessage);
    print("");
    print("[ Retrosim alpha ]");
    snprintf(textBuffer, TBL, "Mem:..................%d KBytes (%d MBytes)", core->mmu->MAXMEM / 1024, core->mmu->MAXMEM / 1024 / 1024); print(textBuffer);
    print("CPU...................A65000");
    snprintf(textBuffer, TBL, "gpu...................G1 (at $%X)", core->GPU_REG_BASE); print(textBuffer); print(""); print("[ System Vectors ]");
    snprintf(textBuffer, TBL, "[$%.4X] Reset................ $%.8X", a65kcpu->VEC_RESET, core->mmu->read<uint32>(a65kcpu->VEC_RESET)); print(textBuffer);
    snprintf(textBuffer, TBL, "[$%.4X] Stack................ $%.8X", a65kcpu->VEC_STACKPOINTERINIT, core->mmu->read<uint32>(a65kcpu->VEC_STACKPOINTERINIT)); print(textBuffer);
    snprintf(textBuffer, TBL, "[$%.4X] IRQ.................. $%.8X", a65kcpu->VEC_HWIRQ, core->mmu->read<uint32>(a65kcpu->VEC_HWIRQ)); print(textBuffer);
    snprintf(textBuffer, TBL, "[$%.4X] Software IRQ......... $%.8X", a65kcpu->VEC_SOFTIRQ, core->mmu->read<uint32>(a65kcpu->VEC_SOFTIRQ)); print(textBuffer);
    snprintf(textBuffer, TBL, "[$%.4X] NMI.................. $%.8X", a65kcpu->VEC_NMI, core->mmu->read<uint32>(a65kcpu->VEC_NMI)); print(textBuffer);
    snprintf(textBuffer, TBL, "[$%.4X] Illegal Instruction.. $%.8X", a65kcpu->VEC_ILLEGALINSTRUCTION, core->mmu->read<uint32>(a65kcpu->VEC_ILLEGALINSTRUCTION)); print(textBuffer);
    snprintf(textBuffer, TBL, "[$%.4X] Illegal Memory Access $%.8X", a65kcpu->VEC_ILLEGALADDRESS, core->mmu->read<uint32>(a65kcpu->VEC_ILLEGALADDRESS)); print(textBuffer);
    snprintf(textBuffer, TBL, "[$%.4X] Division by Zero..... $%.8X", a65kcpu->VEC_DIVZERO, core->mmu->read<uint32>(a65kcpu->VEC_DIVZERO)); print(textBuffer);
}

// ----------------------------------------------------------------------------

void RSMonitorInterpreter::displayHelp(const vector<string> &arguments)
{
    if(arguments.size() == 1)
    {
        MonitorCommand cmd = recognizeCommand(arguments[0]);
        
        switch(cmd)
        {
            case MonitorCommand::RegisterDump:
                print("Display CPU registers");
                break;
            case MonitorCommand::DisplayGPURegisters:
                print("Display GPU registers");
                break;
            case MonitorCommand::DisplayGPUSurfaceRegisters:
                displayGPUSurfaceHelp();
                break;
            case MonitorCommand::DisplayMemoryByte:
            case MonitorCommand::DisplayMemoryWord:
            case MonitorCommand::DisplayMemoryDWord:
                displayMemoryHelp();
                break;
            case MonitorCommand::DisplayASCII:
                displayAsciiHelp();
                break;
            case MonitorCommand::Disassemble:
                break;
            case MonitorCommand::Run:
                break;
            case MonitorCommand::Quit:
                print("Quit Retrosim");
                break;
            case MonitorCommand::Breakpoint:
                break;
            case MonitorCommand::Invalid:
                break;
            case MonitorCommand::DisplayHelp:
                break;
            case MonitorCommand::ClearScreen:
                print("Clear the screen");
                break;
            case MonitorCommand::DisplaySystemConfig:
                print("Display system information, system vectors");
                break;
            case MonitorCommand::RepeatLastCommand:
                print("Repeat last command");
                break;
            case MonitorCommand::SetValue:
                displaySetValueHelp();
                break;
            case MonitorCommand::ChangeNumericModeToDec:
                print("Change numeric mode to decimal");
                break;
            case MonitorCommand::ChangeNumericModeToHex:
                print("Change numeric mode to hexadecimal");
                break;
            case MonitorCommand::LoadRSX:
                print("Load Retrosim RSX file into memory");
                break;
            case MonitorCommand::LoadBinary:
                displayLoadBinaryHelp();
                break;
            case MonitorCommand::SaveBinary:
                displaySaveBinaryHelp();
                break;
        }
        
        return;
    }
    
    print("[Retrosim Debugger]");
    print("Available commands: (long format in parentheses)");
    print(" sys  Display system information, system vectors (system)");
    print(" r    Display CPU registers (regs)");
    print(" rg   Display GPU registers (gpuregs)");
    print(" rgs  Display GPU surface registers (surfaceregs)");
    print(" m    Display memory contents in byte format (mem)");
    print(" mw   Display memory contents in word format (memword)");
    print(" md   Display memory contents in double-word format (memdword)");
    print(" i    Display memory contents in alphanumeric format (ascii)");
    print(" d    Display disassembly");
    print(" set  Change register/memory value [setb/setw/setd]");
    print(" dec  Change default numeric mode to decimal");
    print(" hex  Change default numeric mode to hexadecimal");
    print(" l    Load Retrosim RSX file into memory (load)");
    print(" lb   Load binary file into memory (loadbinary)");
    print(" s    Save binary data from memory (save)");
    print(" g    Run code from specified memory address (run)");
    print(" cls  Clear screen");
    print(" .    Repeat last command");
    print(" q    Quit Retrosim");
    print("For additional info, enter 'help <command>'");
}

// ----------------------------------------------------------------------------

void RSMonitorInterpreter::runCode(const vector<string> arguments)
{
    if(arguments.size() == 1)
    {
        uint32 address;
        try
        {
            address = convertStringToInteger(arguments[0]);
        }
        catch(...)
        {
            print("Error: invalid address format");
            return;
        }
        
//        a65kcpu = (A65000CPU *)core->cpu;

//        a65kcpu = dynamic_cast<A65000CPU *>(core->cpu);
  
        core->cpu->registers[15] = 0x1000;
        core->cpu->registers[14] = address - 1;
        core->cpu->registers[13] = address - 2;
//        a65kcpu->PC = address;
//        core->cpu->setPC(address);
        core->cpu->sleep = false;
        
        snprintf(textBuffer, TBL, "Program started at: $%X", address);
        print(textBuffer);
    }
    else
        displayRunHelp();
}

// ----------------------------------------------------------------------------

void RSMonitorInterpreter::displayDisassemblyHelp(const string &errorMessage = "")
{
    print("Show disassembly");
    if(errorMessage != "")
        print(errorMessage);
    print("Usage: d|disasm <start address>");
}

// ----------------------------------------------------------------------------

void RSMonitorInterpreter::displayDisassembly(const vector<string> arguments)
{
    uint8 linesToPrint = 16;

    if(arguments.size() == 1)
    {
        try
        {
            displayDisassemblyAddress = convertStringToInteger(arguments[0]);
        }
        catch (...)
        {
            print("Error: invalid address format");
            return;
        }
    }
    if(arguments.size() > 1)
    {
        displayDisassemblyHelp("Error: invalid number of arguments");
        return;
    }
    
    A65000Disassembler disassembler{};
    A65000Disassembler::Disassembly disassembly;
    
    try
    {
        disassembly = disassembler.getDisassembly(&core->mmu->memPtr[displayDisassemblyAddress], displayDisassemblyAddress, linesToPrint); // TODO: check invalid memory accesses
    }
    catch(A65000Disassembler::Error e)
    {
        print(e.errorString);
        return;
    }
    catch(...)
    {
        print("Disassembly error");
        return;
    }
    
    for(string line : disassembly.text)
    {
        print(line);
    }
    
    displayDisassemblyAddress += disassembly.bytesProcessed;
}

// ----------------------------------------------------------------------------

void RSMonitorInterpreter::displaySaveBinaryHelp(const string &errorMessage)
{
    print("Save binary data from memory");
    if(errorMessage != "")
        print(errorMessage);
    print("Usage: s|save <start address> <end address>");
    print("The <end address> is inclusive");
}

// ----------------------------------------------------------------------------

void RSMonitorInterpreter::saveBinaryFile(const vector<string> &arguments)
{
    if(arguments.size() != 2)
    {
        displaySaveBinaryHelp();
        return;
    }
    
    uint32 startAddress = 0;
    uint32 endAddress = 0;
    
    try
    {
        startAddress = convertStringToInteger(arguments[0]);
        endAddress = convertStringToInteger(arguments[1]);
        if(startAddress >= endAddress)
            throw;
    }
    catch (...)
    {
        snprintf(textBuffer, TBL, "Invalid memory address");
        displaySaveBinaryHelp(textBuffer);
        return;
    }
    
    nfdchar_t *savePath = nullptr;
    nfdresult_t nfd_result = NFD_SaveDialog(nullptr, nullptr, &savePath);
    if (nfd_result == NFD_ERROR)
    {
        print("Error showing \"save file\" dialog");
        return;
    }
    if (nfd_result == NFD_CANCEL)
    {
        print("Saving cancelled");
        return;
    }
    
    FILE *file = fopen(savePath, "w");
    if (file)
    {
        uint32 size = endAddress - startAddress;
        
        snprintf(textBuffer, TBL, "Filesize: $%X (%u) bytes", size, size);
        print(textBuffer);
        /*
         if((loadAddress + size) >= core->mmu->MAXMEM)
         {
         //                snprintf(textBuffer, TBL, "Error: file (%u bytes) won't fit into memory at address %.8X", (uint32)size, loadAddress);
         snprintf(textBuffer, TBL, "Error: illegal memory range for load operation");
         print(textBuffer);
         snprintf(textBuffer, TBL, "Intended load range: [$%X-$%llX]", loadAddress, loadAddress+size);
         print(textBuffer);
         fclose(file);
         return;
         }
         */
        uint8 *memblock = &core->mmu->memPtr[startAddress];
        
        uint32 result = (uint32)fwrite(memblock, 1, size+1, file);
        if(result != size + 1)
        {
            snprintf(textBuffer, TBL, "Error while writing file %s", savePath);
            print(textBuffer);
            fclose(file);
            delete [] memblock;
            return;
        }
        
        fclose(file);
        
        snprintf(textBuffer, TBL, "Saving file: '%s'", savePath);
        print(textBuffer);
        snprintf(textBuffer, TBL, "Memory range [$%X-$%X]", startAddress, endAddress);
        print(textBuffer);
    }
    else
    {
        snprintf(textBuffer, TBL, "Error while opening file %s", savePath);
        print(textBuffer);
    }
}

// ----------------------------------------------------------------------------

void RSMonitorInterpreter::displayLoadBinaryHelp(const string &errorMessage)
{
    print("Load binary file into memory");
    if(errorMessage != "")
        print(errorMessage);
    print("Usage: lb|loadbin <address>");
}

// ----------------------------------------------------------------------------

void RSMonitorInterpreter::loadBinaryFile(const vector<string> &arguments)
{
    if(arguments.size() != 1)
    {
        displayLoadBinaryHelp();
        return;
    }
    
    uint32 loadAddress = 0;
    
    try
    {
        loadAddress = convertStringToInteger(arguments[0]);
    }
    catch (...)
    {
        snprintf(textBuffer, TBL, "Invalid memory address '%s'", arguments[0].c_str());
        displayLoadBinaryHelp(textBuffer);
        return;
    }
    
    nfdchar_t *loadPath = nullptr;
    nfdresult_t nfd_result = NFD_OpenDialog(nullptr, nullptr, &loadPath);
    if (nfd_result == NFD_ERROR)
    {
        print("Error showing \"open file\" dialog");
        return;
    }
    if (nfd_result == NFD_CANCEL)
    {
        print("Loading cancelled");
        return;
    }
    
    FILE *file = fopen(loadPath, "r");
    if (file)
    {
        fseek(file, 0, SEEK_END);
        uint64 size = ftell(file);
        fseek(file, 0, SEEK_SET);
        
        snprintf(textBuffer, TBL, "Filesize: $%llX (%llu) bytes", size, size);
        print(textBuffer);
        
        if((loadAddress + size) >= core->mmu->MAXMEM)
        {
            snprintf(textBuffer, TBL, "Error: file (%u bytes) won't fit into memory at address %.8X", (uint32)size, loadAddress);
            //                snprintf(textBuffer, TBL, "Error: illegal memory range for load operation");
            print(textBuffer);
            snprintf(textBuffer, TBL, "Intended load range: [$%X-$%llX]", loadAddress, loadAddress+size);
            print(textBuffer);
            fclose(file);
            return;
        }
        
        uint8 *memblock = new uint8[size];
        
        uint32 result = (uint32)fread(memblock, 1, size, file);
        if(result != size)
        {
            snprintf(textBuffer, TBL, "Error while reading file %s", loadPath);
            print(textBuffer);
            fclose(file);
            delete [] memblock;
            return;
        }
        
        fclose(file);
        
        uint32 endAddress = loadAddress + (uint32)size;
        uint32 i = 0;
        uint32 addr = loadAddress;
        while(addr < endAddress)
        {
            try
            {
                core->mmu->write(addr++, (uint8)memblock[i++]);
            }
            catch (A65000Exception e)
            {
                invalidAccessError<uint8>(e);
            }
        }
        
        delete[] memblock;
        
        snprintf(textBuffer, TBL, "Loading file: '%s'", loadPath);
        print(textBuffer);
        snprintf(textBuffer, TBL, "Loaded into [$%X-$%llX]", loadAddress, loadAddress+size);
        print(textBuffer);
    }
    else
    {
        snprintf(textBuffer, TBL, "Error while opening file %s", loadPath);
        print(textBuffer);
    }
}

// ----------------------------------------------------------------------------

void RSMonitorInterpreter::displayLoadRSXHelp()
{
    print("Load RSX file");
    print("Usage: l|load <filename>");
    print("<filename> is optional.");
    print("If <filename> is not specified, a file selector dialog window will appear.");
    print("Otherwise, the specified file will be loaded.");
}

// ----------------------------------------------------------------------------

void RSMonitorInterpreter::loadRSXFile(const vector<string> &arguments)
{
    nfdchar_t *loadPath = nullptr;
    
    if(arguments.size() > 1)
    {
        displayLoadRSXHelp();
        return;
    }
    
    if(arguments.size() == 1)
    {
        loadPath = (char *)arguments[0].c_str();
    }
    else
    {
        nfdresult_t nfd_result = NFD_OpenDialog(nullptr, nullptr, &loadPath);
        if (nfd_result == NFD_ERROR)
        {
            print("Error showing \"open file\" dialog");
            return;
        }
        if (nfd_result == NFD_CANCEL)
        {
            print("Loading cancelled");
            return;
        }
    }
    FILE *file = fopen(loadPath, "r");
    if (file == nullptr)
    {
        snprintf(textBuffer, TBL, "Error while opening file %s", loadPath);
        print(textBuffer);
        return;
    }
    
    fseek(file, 0, SEEK_END);
    uint64 fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    snprintf(textBuffer, TBL, "Filesize: $%llX (%llu) bytes", fileSize, fileSize);
    print(textBuffer);
    
    uint8 *buffer = new uint8[fileSize];
    
    snprintf(textBuffer, TBL, "Loading '%s'", loadPath);
    print(textBuffer);
    
    uint32 result = (uint32)fread(buffer, 1, fileSize, file);
    if(result != fileSize)
    {
        snprintf(textBuffer, TBL, "Error while reading file %s", loadPath);
        print(textBuffer);
        fclose(file);
        delete [] buffer;
        return;
    }
    
    fclose(file);
    
    bool invalidFileFormat = false;
    
    if(fileSize > 3)
    {
        string header;
        header.push_back((char)*buffer);
        header.push_back((char)*(buffer+1));
        header.push_back((char)*(buffer+2));
        header.push_back((char)*(buffer+3));
        if(header != "RSX0")
        {
            invalidFileFormat = true;
        }
    }
    else
    {
        invalidFileFormat = true;
    }
    
    if(invalidFileFormat)
    {
        print("Invalid file format: RSX format expected");
        delete [] buffer;
        return;
    }
    
    uint32 bootAddress = 0;
    uint32 i = 4;
    int chunk = 0;
    while(1)
    {
        if(i >= (fileSize-1)) break;
        
        uint32 address = *(uint32 *)&buffer[i];
        i += 4;
        uint32 length = *(uint32 *)&buffer[i];
        i += 4;
        
        if(chunk == 0)
            bootAddress = address;
        
        if((length + address) > core->mmu->MAXMEM)
        {
            print("Error: invalid range for chunk");
            return;
        }
        
        uint32 endAddress = address + (uint32)length;
        //uint32 j = 12; // size of the header
        uint32 addr = address;
        while(addr < endAddress)
        {
            try
            {
                if(i>=fileSize)
                {
                    print("Error: illegal memory access during parsing of the RSX file");
                    return;
                }
                core->mmu->write(addr++, (uint8)buffer[i++]);
            }
            catch (A65000Exception e)
            {
                invalidAccessError<uint8>(e);
            }
        }
        
        snprintf(textBuffer, TBL, "Loaded $%X (%d) bytes into [$%X-$%X]", length, length, address, address+length-1);
        print(textBuffer);
        
        //i += length;
        chunk++;
    }
    
    snprintf(textBuffer, TBL, "Code start address: $%X", bootAddress);
    print(textBuffer);
}

// ----------------------------------------------------------------------------

void RSMonitorInterpreter::changeNumericModeToDec() // TODO: no need to make a function for this line
{
    currentNumericMode = NumericMode::dec;
}

// ----------------------------------------------------------------------------

void RSMonitorInterpreter::changeNumericModeToHex()
{
    currentNumericMode = NumericMode::hex;
}

// ----------------------------------------------------------------------------

void RSMonitorInterpreter::clearScreen() // TODO: no need for a function for this line
{
    windowBuffer->clear();
}

// ----------------------------------------------------------------------------

void RSMonitorInterpreter::displayGPUSurfaceHelp(const char *errorMessage)
{
    print("Display Surface Properties");
    if(errorMessage)
        print(errorMessage);
    print("Usage: rgs|surfaceregs <surface index>");
}

// ----------------------------------------------------------------------------

void RSMonitorInterpreter::displayGPUSurfaceRegisters(const vector<string> &arguments)
{
    if(arguments.size() > 1)
    {
        displayGPUSurfaceHelp();
        return;
    }
    
    uint16 surfaceIndex = 0;
    
    if(arguments.size() == 1)
    {
        try
        {
            surfaceIndex = convertStringToInteger(arguments[0]);
        }
        catch (...)
        {
            snprintf(textBuffer, TBL, "Invalid parameter '%s'", arguments[0].c_str());
            displayGPUSurfaceHelp(textBuffer);
            return;
        }
    }
   
    GPU_G1::GPURegisters *gpuRegs = (GPU_G1::GPURegisters *)(core->mmu->memPtr+(core->GPU_REG_BASE));
    GPU_G1::GenericSurface *surfacePtr = (GPU_G1::GenericSurface *)(core->mmu->memPtr+gpuRegs->SurfacesPtr+(surfaceIndex * 0x100));
    
    auto boolToString = [](bool input) // lambda function
    {
        if(input) return string("True");
        else return string("False");
    };
    
    snprintf(textBuffer, TBL, "Surface #%d", surfaceIndex); print(textBuffer); //print("");
    snprintf(textBuffer, TBL, "[$%X] type           = $%.2X (%s)", gpuRegs->SurfacesPtr+(surfaceIndex * 0x100), surfacePtr->type, core->gpu->getEnumStringSurfaceType((GPU_G1::SurfaceType)surfacePtr->type).c_str()); print(textBuffer);
    snprintf(textBuffer, TBL, "[$%X] isHidden       = $%.2X (%s)", gpuRegs->SurfacesPtr+(surfaceIndex * 0x100) + 1, surfacePtr->isHidden, boolToString(surfacePtr->isHidden).c_str()); print(textBuffer);
    snprintf(textBuffer, TBL, "[$%X] width          = $%.4X (%d)", gpuRegs->SurfacesPtr+(surfaceIndex * 0x100) + 2, surfacePtr->width, surfacePtr->width); print(textBuffer);
    snprintf(textBuffer, TBL, "[$%X] height         = $%.4X (%d)", gpuRegs->SurfacesPtr+(surfaceIndex * 0x100) + 4, surfacePtr->height, surfacePtr->height); print(textBuffer);
    snprintf(textBuffer, TBL, "[$%X] xPos           = $%.8X (%d)", gpuRegs->SurfacesPtr+(surfaceIndex * 0x100) + 6, surfacePtr->xPos, surfacePtr->xPos); print(textBuffer);
    snprintf(textBuffer, TBL, "[$%X] yPos           = $%.8X (%d)", gpuRegs->SurfacesPtr+(surfaceIndex * 0x100) + 10, surfacePtr->yPos, surfacePtr->yPos); print(textBuffer);
    snprintf(textBuffer, TBL, "[$%X] framebufferPtr = $%.8X (%d)", gpuRegs->SurfacesPtr+(surfaceIndex * 0x100) + 14, surfacePtr->framebufferPtr, surfacePtr->framebufferPtr); print(textBuffer);
}

// ----------------------------------------------------------------------------

void RSMonitorInterpreter::displayGPURegisters()
{
    GPU_G1::GPURegisters *gpuRegs = (GPU_G1::GPURegisters *)(core->mmu->memPtr+(core->GPU_REG_BASE));
    
    print("GPU Registers");
    snprintf(textBuffer, TBL, "[$%X] renderMode  = $%.2X   (%s)", core->GPU_REG_BASE, gpuRegs->renderMode, core->gpu->getEnumStringRenderMode((GPU_G1::RenderMode)gpuRegs->renderMode).c_str()); print(textBuffer);
    snprintf(textBuffer, TBL, "[$%X] stageWidth  = $%.4X (%d)", core->GPU_REG_BASE + 1, gpuRegs->stageWidth, gpuRegs->stageWidth); print(textBuffer);
    snprintf(textBuffer, TBL, "[$%X] stageHeight = $%.4X (%d)", core->GPU_REG_BASE + 3, gpuRegs->stageHeight, gpuRegs->stageHeight); print(textBuffer);
    snprintf(textBuffer, TBL, "[$%X] currentX    = $%.4X (%d) [read only]", core->GPU_REG_BASE + 5, gpuRegs->currentX, gpuRegs->currentX); print(textBuffer);
    snprintf(textBuffer, TBL, "[$%X] currentY    = $%.4X (%d) [read only]", core->GPU_REG_BASE + 7, gpuRegs->currentY, gpuRegs->currentY); print(textBuffer);
    snprintf(textBuffer, TBL, "[$%X] numSurfaces = $%.4X (%d)", core->GPU_REG_BASE + 9, gpuRegs->numSurfaces, gpuRegs->numSurfaces); print(textBuffer);
    snprintf(textBuffer, TBL, "[$%X] SurfacesPtr = $%.8X (%d)", core->GPU_REG_BASE + 13, gpuRegs->SurfacesPtr, gpuRegs->SurfacesPtr); print(textBuffer);
}

// ----------------------------------------------------------------------------

void RSMonitorInterpreter::displaySetValueHelp(const string errorMessage)
{
    print("Change register/memory value");
    if(errorMessage != "")
        print("Error: " + errorMessage);
    print("Usage: set[b|w|d] <register|address> <value>");
    print("<register> = r0..r14, SP, PC");
    print("The size specifiers (b/w/d) are obligatory.");
}

// ----------------------------------------------------------------------------

template <class T>
void RSMonitorInterpreter::setValue(const vector<string> &arguments)
{
    if(arguments.size() == 0)
    {
        displaySetValueHelp();
        return;
    }
    
    static const regex rx_detectRegisterIndex(R"(r([0-9]{1,2})|pc|sp)");
    smatch registerIndexMatch;
    if(regex_match(arguments[0], registerIndexMatch, rx_detectRegisterIndex) == true)
        setRegister<T>(arguments);
    else
        setMemory<T>(arguments);
}

// ----------------------------------------------------------------------------

template <class T>
void RSMonitorInterpreter::setRegister(const vector<string> &arguments) // TODO: shall this modify the flags? A: NO
{
    if(arguments.size() !=2 )
    {
        displaySetValueHelp("invalid number of arguments");
        return;
    }
    
    uint8 registerIndex = 0;
    string registerString = arguments[0];
    
    if(registerString == "pc")
        registerIndex = 15;
    if(registerString == "sp")
        registerIndex = 14;
    
    if(registerIndex == 0)
    {
        static const regex rx_detectRegisterIndex(R"(r([0-9]{1,2}))");
        smatch registerIndexMatch;
        if(regex_match(registerString, registerIndexMatch, rx_detectRegisterIndex) == false)
        {
            displaySetValueHelp("invalid register");
            return;
        }
        
        try {
            registerIndex = (uint8)std::stoi(registerIndexMatch[1].str());
        } catch (...)
        {
            displaySetValueHelp("invalid register");
            return;
        }
        
        if(registerIndex > 13)
        {
            displaySetValueHelp("invalid register");
            return;
        }
    }
    
    uint32 value;
    
    try
    {
        value = convertStringToInteger(arguments[1]);
    }
    catch(...)
    {
        displaySetValueHelp("invalid value");
        return;
    }
    
    if(value > (T)-1)
    {
        snprintf(textBuffer, TBL, "Value out of range for given target size. Maximum = $%X (%d)", (T)-1, (T)-1);
        print(textBuffer);
    }
    
    union
    {
        T newValue;
        uint32 reg;
    }newReg;
    
    newReg.reg = a65kcpu->registers[registerIndex];
    newReg.newValue = (T)value; // TODO: verify this
    a65kcpu->registers[registerIndex] = newReg.reg; // wow, high five! :D
}

// ----------------------------------------------------------------------------

template <class T>
void RSMonitorInterpreter::setMemory(const vector<string> &arguments)
{
    if(arguments.size() !=2 )
    {
        displaySetValueHelp("invalid number of arguments");
        return;
    }
    
    uint32 address;
    
    try
    {
        address = convertStringToInteger(arguments[0]);
    }
    catch(...)
    {
        print("invalid memory address");
        return;
    }
    
    
    uint32 value;
    
    try
    {
        value = convertStringToInteger(arguments[1]);
    }
    catch(...)
    {
        displaySetValueHelp("invalid value");
        return;
    }
    
    if(value > (T)-1)
    {
        snprintf(textBuffer, TBL, "Value out of range for given target size. Maximum = $%X (%d)", (T)-1, (T)-1);
        print(textBuffer);
    }
    
    try
    {
        core->mmu->write(address, (T)value);
    }
    catch (A65000Exception e)
    {
        invalidAccessError<T>(e);
    }
}

// ----------------------------------------------------------------------------

void RSMonitorInterpreter::displayAsciiHelp(const string &errorMessage)
{
    print("Display memory contents in alphanumeric format");
    if(errorMessage != "")
        print(errorMessage);
    print("Usage: i|ascii <memory address>");
}

// ----------------------------------------------------------------------------

string RSMonitorInterpreter::getNthUTF8Character(const string &input, uint8 n)
{
    string output;
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> cvt;
    
    // UTF-8 to UTF-32
    std::u32string utf32 = cvt.from_bytes(input);
    std::u32string selectedChar = utf32.substr(n, 1);
    // UTF-32 to UTF-8
    output = cvt.to_bytes(selectedChar);
    
    return output;
}

// ----------------------------------------------------------------------------

void RSMonitorInterpreter::displayAscii(vector<string> arguments)
{
    bool showInvisibles = false;
    
    // if argument "showInvisibles" is found, set flag and remove argument to remain transparent
    if(find(arguments.begin(), arguments.end(), "showInvisibles") != arguments.end())
    {
        showInvisibles = true;
        arguments.erase(find(arguments.begin(), arguments.end(), "showInvisibles"));
    }
    
    if(arguments.size() > 0)
    {
        if(arguments.size() == 1)
        {
            try
            {
                displayAsciiAddress = convertStringToInteger(arguments[0]);
            }
            catch (...)
            {
                print("Error: invalid address");
                return;
            }
        }
        else
        {
            displayAsciiHelp();
            return;
        }
    }
    
    const string invisibleChars = "ΘαβγδεζηθικλμνξοπρστυφχψωΓΔΛΞΠΣΨΩ";
    
    for(int line=0; line<8; line++)
    {
        string dataBuffer;
        
        const int maxrows = 64;
        for(int row = 0; row < maxrows; row++)
        {
            uint8 value = 0;
            try
            {
                value = core->mmu->read<uint8>(displayAsciiAddress + row);
            }
            catch (A65000Exception e)
            {
                invalidAccessError<uint8>(e);
                return;
            }
            
            if(value < 32 || value > 126)
            {
                if(showInvisibles == true) // replace non-visible characters with Greek symbols
                    dataBuffer += getNthUTF8Character(invisibleChars, value%32);
                else
                    dataBuffer += ".";
            }
            else
                dataBuffer += value;
        }
        
        snprintf(textBuffer, TBL, "%.8x: %s", displayAsciiAddress, dataBuffer.c_str());
        
        print(string(textBuffer));
        
        displayAsciiAddress += maxrows;
    }
}

// ----------------------------------------------------------------------------

vector<string> RSMonitorInterpreter::split(const char *str, char c)
{
    vector<string> result;
    do {
        const char *begin = str;
        
        while(*str != c && *str)
            str++;
        
        result.push_back(string(begin, str));
    } while (0 != *str++);
    
    return result;
}

// ----------------------------------------------------------------------------

void RSMonitorInterpreter::displayRegisters()
{
    string actLine;
    for(int i=0; i<16; i++)
    {
        char *actLineCstr = new char[TBL];
        
        if(i==14) snprintf(actLineCstr, TBL, "SP  = $%.8X  ", a65kcpu->registers[i]);
        else if(i==15) snprintf(actLineCstr, TBL, "PC  = $%.8X  ", a65kcpu->registers[i]);
        else snprintf(actLineCstr, TBL, "r%.2d = $%.8X  ", i, a65kcpu->registers[i]);
        
        actLine += string(actLineCstr);
        if(!((i+1)%4))
        {
            print(actLine);
            actLine = "";
        }
        
        delete [] actLineCstr;
    }
    
    //        core->cpu->statusRegister.z = 1;
    //        core->cpu->statusRegister.n = 0;
    //        core->cpu->statusRegister.c = 1;
    //        core->cpu->statusRegister.v = 1;
    //        core->cpu->statusRegister.b = 1;
    //        core->cpu->statusRegister.i = 1;
    
    uint8 status = *(uint8 *)(&a65kcpu->statusRegister);
    string statusString;
    
    for(int i=7; i>=0; i--)
    {
        bool flag = (status >> i) & 1;
        statusString += flag?"1":"0";
    }
    
    char actLineStr[TBL];
    snprintf(actLineStr, TBL, "%.2X", status);
    
    statusString += " ($" + string(actLineStr) + ")";
    
    print("Status --IBVCNZ");
    print("Flags: " + statusString);
}

// ----------------------------------------------------------------------------

template <class T>
void RSMonitorInterpreter::invalidAccessError(A65000Exception e)
{
    if(e.type == EX_INVALID_ADDRESS)
    {
        string errorMessage;
        
        snprintf(textBuffer, TBL, "%.8llx", e.address);
        
        errorMessage.append("MMU Exception: invalid access (");
        if(sizeof(T) == 1)
            errorMessage.append("8");
        if(sizeof(T) == 2)
            errorMessage.append("16");
        if(sizeof(T) == 4)
            errorMessage.append("32");
        errorMessage.append("-bit read at address: $");
        errorMessage.append(string(textBuffer));
        errorMessage.append(")");
        print(errorMessage);
    }
    else
        print("MMU Exception: internal error");
}

// ----------------------------------------------------------------------------

void RSMonitorInterpreter::displayMemoryHelp()
{
    print("Display memory contents");
    print("Usage: mem[d|w] <address>");
    print("<address> is optional (continues print from last time)");
}

// ----------------------------------------------------------------------------

void RSMonitorInterpreter::displayMemory(const vector<string> &arguments, Base base)
{
    if(arguments.size() == 1)
    {
        if(arguments[0] == "h" || arguments[0] == "help")
        {
            displayMemoryHelp();
            return;
        }
        if(arguments[0] != "")
            displayMemAddress = convertStringToInteger(arguments[0]);
    }
    
    if(arguments.size() > 2)
    {
        displayMemoryHelp();
        return;
    }
    
    switch(base)
    {
        case Base::byte:
            displayMemory<uint8>();
            break;
        case Base::word:
            displayMemory<uint16>();
            break;
        case Base::dword:
            displayMemory<uint32>();
            break;
    }
}

// ............................................................................

template<class T>
void RSMonitorInterpreter::displayMemory()
{
    try
    {
        for(int i=0; i<8; i++)
        {
            if(sizeof(T) == 1) // byte
            {
                char asciibuffer[17];
                for(int i=0; i<16; i++)
                {
                    const char actChar = core->mmu->read<uint8>(displayMemAddress + i);
                    if((actChar < 32) || (actChar > 126))
                        asciibuffer[i] = '.';
                    else
                        asciibuffer[i] = actChar;
                }
                asciibuffer[16] = '\0';
                
                uint8 dataBuffer[16];
                for(int i=0; i<16; i++)
                    dataBuffer[i] = core->mmu->read<uint8>(displayMemAddress + i);
                
                snprintf(textBuffer, TBL, "%.8x: %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x |%s|", displayMemAddress, *dataBuffer, dataBuffer[1], dataBuffer[2], dataBuffer[3], dataBuffer[4], dataBuffer[5], dataBuffer[6], dataBuffer[7], dataBuffer[8], dataBuffer[9], dataBuffer[10], dataBuffer[11], dataBuffer[12], dataBuffer[13], dataBuffer[14], dataBuffer[15], asciibuffer);
                displayMemAddress += 16;
                print(string(textBuffer));
            }
            else
            {
                char asciibuffer[17];
                for(int i=0; i<16; i++)
                {
                    if((core->mmu->read<T>(displayMemAddress+i) < 32) || (core->mmu->read<T>(displayMemAddress+i) > 254))
                        asciibuffer[i] = '.';
                    else
                        asciibuffer[i] = core->mmu->read<T>(displayMemAddress + i);
                }
                asciibuffer[16] = '\0';
                
                T dataBuffer[16];
                for(int i=0; i<16; i++)
                    dataBuffer[i] = core->mmu->read<T>(displayMemAddress + i * sizeof(T));
                
                if(sizeof(T) == 2) // word
                    snprintf(textBuffer, TBL, "%.8x: %.4x %.4x %.4x %.4x %.4x %.4x %.4x %.4x", displayMemAddress, *dataBuffer, dataBuffer[1], dataBuffer[2], dataBuffer[3], dataBuffer[4], dataBuffer[5], dataBuffer[6], dataBuffer[7] );
                else // dword
                    snprintf(textBuffer, TBL, "%.8x: %.8x %.8x %.8x %.8x %.8x %.8x %.8x %.8x", displayMemAddress, *dataBuffer, dataBuffer[1], dataBuffer[2], dataBuffer[3], dataBuffer[4], dataBuffer[5], dataBuffer[6], dataBuffer[7] );
                
                displayMemAddress += 8 * sizeof(T);
                print(string(textBuffer));
            }
        }
    }
    catch (A65000Exception e)
    {
        invalidAccessError<T>(e);
    }
}

// ----------------------------------------------------------------------------

int RSMonitorInterpreter::convertStringToInteger(string valueStr) // TODO: bug: accepts "sdf" as a valid number
{
    static const regex rx_checkIfBin(R"(^%([0-1]+))", std::regex_constants::icase);
    static const regex rx_checkIfHex(R"(^\$([0-9a-z]+))", std::regex_constants::icase);
    static const regex rx_checkIfHexWithoutDollar(R"(^([0-9a-z]+))", std::regex_constants::icase);
    static const regex rx_checkIfDec(R"(^([0-9]+))", std::regex_constants::icase);
    static const string table = "0123456789ABCDEF";
    smatch rxResult;
    int i;
    uint64_t result = 0;
    char actChar;
    int length;
    uint64_t baseValue = 1;
    
    auto findChar = [](const string text, char c)
    {
        const int size = (int)text.size();
        c = toupper(c);
        
        for(int i = 0; i < size; i++)
            if(text[i] == c)
                return i;
        
        return 0;
    };
    
    if(regex_search(valueStr, rxResult, currentNumericMode == NumericMode::dec ? rx_checkIfHex : rx_checkIfHexWithoutDollar))
    {
        string tmpStr = rxResult[1];
        length = (int)tmpStr.length();
        for(i = length-1; i >= 0; i--)
        {
            actChar = tmpStr[i];
            result += findChar(table, actChar) * baseValue;
            baseValue *= 16;
        }
        
        checkIntegerRange(result);
        return (int)result;
    }
    
    if(regex_search(valueStr, rxResult, rx_checkIfBin))
    {
        const string tmpStr = rxResult[1];
        length = (int)tmpStr.length();
        for(i = length-1; i >= 0; i--)
        {
            actChar = tmpStr[i];
            if(actChar=='1')
                result += baseValue;
            baseValue *= 2;
        }
        
        checkIntegerRange(result);
        return (int)result;
    }
    
    if(regex_search(valueStr, rxResult, rx_checkIfDec))
    {
        string tmpStr = rxResult[1];
        length = (int)tmpStr.length();
        for(i = length-1; i >= 0; i--)
        {
            actChar = tmpStr[i];
            result += findChar(table, actChar) * baseValue;
            baseValue *= 10;
        }
        
        checkIntegerRange(result);
        return (int)result;
    }
    
    //throwException_InvalidNumberFormat(); // invalid number format
    throw 1;
    return 0;
}

// ----------------------------------------------------------------------------

void RSMonitorInterpreter::checkIntegerRange(const uint64_t result)
{
    const uint64_t maxInt = 0xffffffff;
    
    if((uint64_t)result > maxInt)
    {
        print("Error: Value exceeding 32 bit range");
    }
}

void RSMonitorInterpreter::displayRunHelp()
{
    print("Run code from specified memory address");
    print("Usage: g|run <address>");
}
