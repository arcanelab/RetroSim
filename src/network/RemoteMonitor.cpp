// RetroSim - Copyright 2011-2023 Zoltán Majoros. All rights reserved.
// https://github.com/arcanelab

#include <cstring>
#include <cstdio>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <unordered_map>
#include "RemoteMonitor.h"
#include "Logger.h"
#include "MMU.h"
#include "Core.h"
#include "A65000Disassembler.h"

using namespace std;
using namespace RetroSim::Logger;

namespace RetroSim::RemoteMonitor
{
    std::unordered_map<string, Command> commands = {
        {"help", displayHelp},
        {"m", displayMemory},
        {"mem", displayMemory},
        {"set8", setMemoryU8},
        {"set16", setMemoryU16},
        {"set32", setMemoryU32},
        {"setf", setMemoryF},
        {"dasm", disassemble},
        {"d", disassemble},
        {"stopGPU", stopGPU},
        {"startGPU", startGPU},
        {"stopCPU", stopCPU},
        {"startCPU", startCPU},
        {"reset", reset},
        {"step", step},
        {"run", run},
        {"quit", quit}};

    string DisplayHelp()
    {
        return "Available commands:\nhelp, mem, set8, set16, set32, setf, dasm (short: d)";
    }

    string DisplayMemoryHelp()
    {
        return "mem <address> [bytes]";
    }

    string DisplayDisassemblyHelp()
    {
        return "d <address> [lines]";
    }

    string DisplayMemory(std::vector<string> tokens)
    {
        size_t numTokens = tokens.size();
        if (numTokens < 2 && numTokens > 3)
        {
            return DisplayMemoryHelp();
        }

        uint32_t address = 0;
        uint32_t bytes = 128;

        try
        {
            address = std::stoul(tokens[1], nullptr, 0);

            if (tokens.size() == 3)
                bytes = std::stoul(tokens[2], nullptr, 0);

            if (bytes < 1)
                throw;
        }
        catch (...)
        {
            return DisplayMemoryHelp();
        }

        if (address > MMU::memorySize || address + bytes > MMU::memorySize)
            return "Invalid address";

        uint32_t rows = bytes / 16;

        std::stringstream ss;
        for (uint32_t row = 0; row < rows; row++)
        {
            ss << std::hex << std::uppercase << std::setfill('0') << std::setw(8) << address + row * 16 << " ";

            for (uint32_t col = 0; col < 16; col++)
            {
                ss << std::hex << std::uppercase << std::setfill('0') << std::setw(2) << (int)MMU::ReadMem<uint8_t>(address + row * 16 + col) << " ";
            }

            ss << " ";

            for (uint32_t col = 0; col < 16; col++)
            {
                uint8_t value = MMU::ReadMem<uint8_t>(address + row * 16 + col);
                if (value >= 32 && value <= 126)
                {
                    ss << (char)value;
                }
                else
                {
                    ss << ".";
                }
            }

            ss << "\n";
        }

        return ss.str();
    }

    template <typename T>
    std::string SetMemory(std::vector<std::string> tokens)
    {
        if (tokens.size() != 3)
        {
            return "set" + std::to_string(sizeof(T) * 8) + " <address> <value>";
        }

        uint32_t address = 0;
        T value = 0;

        try
        {
            address = std::stoul(tokens[1], nullptr, 0);
        }
        catch (...)
        {
            return "Invalid address";
        }

        try
        {
            if (tokens[0] == "setf")
            {
                value = static_cast<T>(std::stof(tokens[2]));
            }
            else
            {
                value = static_cast<T>(std::stoul(tokens[2], nullptr, 0));
            }
        }
        catch (...)
        {
            return "Invalid argument";
        }

        if (address > MMU::memorySize)
            return "Invalid address";

        MMU::WriteMem<T>(address, value);

        std::ostringstream oss;
        oss << "0x" << std::hex << value << " written to 0x" << std::setfill('0') << std::setw(sizeof(uint32_t) * 2) << std::hex << static_cast<uint32_t>(address);

        return oss.str();
    }

    string Reset()
    {
        Core::GetInstance()->Reset();
        return "Resetting.";
    }

    string Disassemble(std::vector<std::string> tokens)
    {
        if (tokens.size() < 2 && tokens.size() > 3)
        {
            return DisplayDisassemblyHelp();
        }

        uint32_t address = 0;
        uint32_t lines = 16;

        try
        {
            address = std::stoul(tokens[1], nullptr, 0);

            if (tokens.size() == 3)
                lines = std::stoul(tokens[2], nullptr, 0);

            if (lines < 1)
                throw;
        }
        catch (...)
        {
            return DisplayDisassemblyHelp();
        }

        if (address > MMU::memorySize)
            return "Invalid address";

        string result;
        A65000Disassembler disasm;
        auto disassembly = disasm.getDisassembly(MMU::memory.raw + address, address, lines);
        for (auto &line : disassembly.text)
        {
            result += line + "\n";
        }

        return result;
    }

    string ProcessCommand(const string &command)
    {
        std::vector<string> tokens;
        std::istringstream iss(command);
        string token;

        while(iss >> token)
            tokens.push_back(token);

        if(tokens.size() == 0)
            return DisplayHelp();

        auto it = commands.find(tokens[0]);
        if (it == commands.end())
        {
            std::string response = "Unknown command: " + tokens[0] + "\n" + DisplayHelp();
            return response;
        }

        switch(it->second)
        {
            case displayHelp:
                return DisplayHelp();
            case displayMemory:
                return DisplayMemory(tokens);
            case setMemoryU8:
                return SetMemory<uint8_t>(tokens);
            case setMemoryU16:
                return SetMemory<uint16_t>(tokens);
            case setMemoryU32:
                return SetMemory<uint32_t>(tokens);
            case setMemoryF:
                return SetMemory<float>(tokens);
            case disassemble:
                return Disassemble(tokens);
            case stopGPU:
                return "stopGPU";
            case startGPU:
                return "startGPU";
            case stopCPU:
                return "stopCPU";
            case startCPU:
                return "startCPU";
            case reset:
                return Reset();
            case step:
                return "step";
            case run:
                return "run";
            case quit:
                return "quit";
            default:
                return "Unknown command";
        }
    }
}
