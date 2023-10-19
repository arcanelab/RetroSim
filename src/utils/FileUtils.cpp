// RetroSim - Copyright 2011-2023 Zoltán Majoros. All rights reserved.
// https://github.com/arcanelab

#include <fstream>
#include <iostream>
#include <sstream>
#include "FileUtils.h"

namespace RetroSim
{
    std::string ReadTextFile(const std::string &filename)
    {
        std::ifstream file(filename);

        if (!file.good())
        {
            std::cout << "Error: couldn't open file " << filename << std::endl;
            return "";
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        file.close();

        return buffer.str();
    }

    uint8_t *ReadBinaryFile(const std::string &filename, size_t &size)
    {
        std::ifstream file(filename, std::ios::binary | std::ios::ate);

        if (!file.good())
        {
            std::cout << "Error: couldn't open file " << filename << std::endl;
            return nullptr;
        }

        size = file.tellg();
        file.seekg(0, std::ios::beg);

        uint8_t *buffer = new uint8_t[size];
        file.read((char*)buffer, size);
        file.close();

        return buffer;
    }

    bool FileExists(const std::string &filename)
    {
        std::ifstream file(filename);
        return file.good();
    }
}
