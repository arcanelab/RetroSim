// RetroSim - Copyright 2011-2023 Zoltán Majoros. All rights reserved.
// https://github.com/arcanelab

#pragma once
#include <string>

namespace RetroSim
{
    std::string ReadTextFile(const std::string &filename);
    uint8_t *ReadBinaryFile(const std::string &filename, size_t &size);
    bool FileExists(const std::string &filename);
    std::string ConvertToWindowsPath(std::string path);
}
