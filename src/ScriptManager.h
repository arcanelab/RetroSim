// RetroSim - Copyright 2011-2023 Zoltán Majoros. All rights reserved.
// https://github.com/arcanelab

#pragma once
#include <string>
#include "gravity_value.h"
#include <vector>
#include <string>

namespace RetroSim::ScriptManager
{
    void Update(float deltaTime);
    void Destroy();
    void CompileScript(std::string _script);
    void RunScript(std::string functionName, std::vector<gravity_value_t> args, const int numArgs);
    void CompileScriptFromFile(std::string filename);
    void RegisterAPIFunctions();
}
