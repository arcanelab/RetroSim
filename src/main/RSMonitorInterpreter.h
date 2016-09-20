//
//  RSMonitorInterpreter.h
//  RetroSim
//
//  Created by Zoltán Majoros on 11/Nov/15.
//  Copyright © 2015 Zoltán Majoros. All rights reserved.
//

#pragma once

#include "RSTypes.h"
#include <vector>
#include <string>

using std::string;
using std::vector;

class A65000Exception;
class A65000CPU;
class RSCore;

class RSMonitorInterpreter
{
    enum NumericMode
    {
        dec, hex
    };
public:
    const string numericModeStrings[2] = {"dec", "hex"};
    NumericMode currentNumericMode = NumericMode::hex;
    
    vector<string> *windowBuffer = nullptr;
    vector<string> processLine(string inputLine);
    
    RSMonitorInterpreter();
    void setCore(RSCore *core);
    
    RSCore *core = nullptr;

private:
    string lastInput;
    #define TBL 128
    char textBuffer[TBL];
    vector<string> output;
    A65000CPU *a65kcpu = nullptr;
    
    enum class MonitorCommand
    {
        RegisterDump, DisplayMemoryByte, DisplayMemoryWord, DisplayMemoryDWord,
        Disassemble, DisplayASCII, Run, Quit, Breakpoint,
        RepeatLastCommand, DisplayHelp, DisplaySystemConfig, ClearScreen,
        DisplayGPUSurfaceRegisters, Invalid, SetValue, DisplayGPURegisters,
        ChangeNumericModeToHex, ChangeNumericModeToDec, LoadRSX, LoadBinary,
        SaveBinary, Step
    };
    
    enum class Base
    {
        byte, word, dword
    };

    uint32 displayMemAddress = 0; // if no arguments given to the 'mem' cmd, continue from here
    string getNthUTF8Character(const string &input, uint8 n);
    uint32 displayAsciiAddress = 0;
    uint32 displayDisassemblyAddress = 0;

    void processCommand(const MonitorCommand cmd, vector<string> arguments, const string &commandStr);
    MonitorCommand recognizeCommand(const string &command);
    void print(const string &text);
    int convertStringToInteger(string valueStr); // TODO: bug: accepts "sdf" as a valid number
    void checkIntegerRange(const uint64_t result);

    // help functions
    void displayLoadBinaryHelp(const string &errorMessage = "");
    void displayDisassemblyHelp(const string &errorMessage);
    void displaySaveBinaryHelp(const string &errorMessage = "");
    void displayGPUSurfaceHelp(const char *errorMessage = nullptr);
    void displaySetValueHelp(const string errorMessage = "");
    void displayAsciiHelp(const string &errorMessage = "");
    void displayMemoryHelp();
    void displayLoadRSXHelp();
    void displayRunHelp();
    
    // monitor commands
    void step();
    void displayGPUSurfaceRegisters(const vector<string> &arguments);
    vector<string> split(const char *str, char c = ' ');
    void displayRegisters();
    void displayMemory(const vector<string> &arguments, Base base);
    void displayAscii(vector<string> arguments);
    void displayGPURegisters();
    void loadBinaryFile(const vector<string> &arguments);
    void loadRSXFile(const vector<string> &arguments);
    void displayDisassembly(const vector<string> arguments);
    void displayHelp(const vector<string> &arguments);
    void displaySystemConfig();
    void runCode(const vector<string> arguments);
    void saveBinaryFile(const vector<string> &arguments);
    void changeNumericModeToDec();
    void changeNumericModeToHex();
    void clearScreen();
    template <class T> void displayMemory();
    template <class T> void setValue(const vector<string> &arguments);
    template <class T> void setRegister(const vector<string> &arguments); // TODO: shall this modify the flags? A: NO
    template <class T> void setMemory(const vector<string> &arguments);

    template <class T> void invalidAccessError(A65000Exception e);
};
