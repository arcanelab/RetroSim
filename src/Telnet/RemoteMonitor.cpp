// RetroSim - Copyright 2011-2023 Zoltán Majoros. All rights reserved.
// https://github.com/arcanelab

#include <cstring>
#include <cstdio>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include "RemoteMonitor.h"
#include "Logger.h"
#include "MMU.h"

using namespace std;
using namespace RetroSim::Logger;

namespace RetroSim::RemoteMonitor
{
    std::unordered_map<string, Command> commands = {
        {"help", displayHelp},
        {"mem", displayMemory},
        {"set8", setMemoryU8},
        {"set16", setMemoryU16},
        {"set32", setMemoryU32},
    };

    string DisplayHelp()
    {
        return "help, mem, set8, set16, set32";
    }

    string DisplayMemoryHelp()
    {
        return "mem <address> [bytes]";
    }

    string DisplayMemory(std::vector<string> tokens)
    {
        if (tokens.size() < 2 && tokens.size() > 3)
        {
            return DisplayMemoryHelp();
        }

        uint32_t address = 0;
        uint32_t bytes = 128;

        try
        {
            address = std::stoi(tokens[1], nullptr, 0);

            if (tokens.size() == 3)
                bytes = std::stoi(tokens[2], nullptr, 0);

            if (bytes < 1)
                throw;
        }
        catch (...)
        {
            return "Invalid argument";
        }

        if (address + bytes > MMU::memorySize)
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

    string SetMemoryU8(std::vector<string> tokens)
    {
        if (tokens.size() != 3)
        {
            return "set8 <address> <value>";
        }

        uint32_t address = 0;
        uint8_t value = 0;

        try
        {
            address = std::stoul(tokens[1], nullptr, 16);
            value = std::stoul(tokens[2], nullptr, 16);
        }
        catch (...)
        {
            return "Invalid argument";
        }

        if (address > MMU::memorySize)
            return "Invalid address";

        MMU::WriteMem<uint8_t>(address, value);

        return tokens[2] + " written to " + tokens[1];
    }

    string ProcessCommand(const string &command)
    {
        LogPrintf(RETRO_LOG_INFO, "RemoteMonitor: %s\n", command.c_str());

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
            return DisplayHelp();
        }

        switch(it->second)
        {
            case displayHelp:
                return DisplayHelp();
            case displayMemory:
                return DisplayMemory(tokens);
            case setMemoryU8:
                return "setMemoryU8";
            case setMemoryU16:
                return "setMemoryU16";
            case setMemoryU32:
                return "setMemoryU32";
        }
    }
}
