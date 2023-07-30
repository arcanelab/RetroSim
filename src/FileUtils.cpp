// RetroSim - Copyright 2011-2023 Zoltán Majoros. All rights reserved.
// https://github.com/arcanelab

#include <fstream>
#include <iostream>
#include <sstream>
#include "FileUtils.h"
#include <SDL.h>

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

    bool FileExists(const std::string &filename)
    {
        std::ifstream file(filename);
        return file.good();
    }
}
