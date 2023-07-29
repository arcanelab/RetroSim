// RetroSim - Copyright 2011-2023 Zoltán Majoros. All rights reserved.
// https://github.com/arcanelab

#pragma once
#include <string>

namespace RetroSim
{
    std::string ReadTextFile(const std::string &filename);
    bool FileExists(const std::string &filename);
}
