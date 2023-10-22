// RetroSim - Copyright 2011-2023 Zoltán Majoros. All rights reserved.
// https://github.com/arcanelab

#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>
#include "FileUtils.h"
#include "Logger.h"

using namespace RetroSim::Logger;

namespace RetroSim
{
    std::string ReadTextFile(const std::string &filename)
    {
        std::string path = ConvertPathToPlatformCompatibleFormat(filename);
        std::ifstream file(path);

        if (!file.good())
        {
            LogPrintf(RETRO_LOG_ERROR, "Error: couldn't open text file %s\n", path.c_str());
            return "";
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        file.close();

        LogPrintf(RETRO_LOG_INFO, "Loaded text file: %s, length = %d\n", path.c_str(), buffer.str().length());
        return buffer.str();
    }

    uint8_t *ReadBinaryFile(const std::string &filename, size_t &size)
    {
        std::ifstream file(ConvertPathToPlatformCompatibleFormat(filename), std::ios::binary | std::ios::ate);

        if (!file.good())
        {
            LogPrintf(RETRO_LOG_ERROR, "Error: couldn't open file %s\n", filename.c_str());
            return nullptr;
        }

        size = file.tellg();
        file.seekg(0, std::ios::beg);

        uint8_t *buffer = new uint8_t[size];
        file.read((char *)buffer, size);
        file.close();

        LogPrintf(RETRO_LOG_INFO, "Loaded file: %s, size: %d\n", filename.c_str(), size);

        return buffer;
    }

    bool FileExists(const std::string &filename)
    {
        std::ifstream file(filename);
        return file.good();
    }

    std::string ConvertPathToPlatformCompatibleFormat(const std::string path)
    {
#ifdef WIN32
        std::string result = path;
        std::replace(result.begin(), result.end(), '/', '\\');
        return result;
#else
        return path;
#endif
    }
}
