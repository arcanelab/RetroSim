// RetroSim - Copyright 2011-2023 Zoltán Majoros. All rights reserved.
// https://github.com/arcanelab

#include "ScriptManager.h"
#include "FileUtils.h"
#include <string>

// Compiles the script and transfers it to the VM
void ScriptManager::CompileScript(std::string script)
{
    this->script = script;

    const char *scriptCStr = script.c_str();
    closure = gravity_compiler_run(compiler, scriptCStr, (uint32_t)strlen(scriptCStr), 0, true, true);
    gravity_compiler_transfer(compiler, vm);
}

void ScriptManager::CompileScriptFromFile(std::string filename)
{
    script = RetroSim::ReadTextFile(filename);
    if(script.empty())
        return;

    CompileScript(script);
    gravity_vm_loadclosure(vm, closure);
}

void ScriptManager::RunScript(std::string functionName, std::vector<gravity_value_t> args, const int numArgs)
{
    gravity_value_t function = gravity_vm_getvalue(vm, functionName.c_str(), strlen(functionName.c_str()));

    gravity_closure_t *closure = VALUE_AS_CLOSURE(function);
    gravity_value_t params[16];
    for (int i = 0; i < numArgs; i++)
    {
        params[i] = args[i];
    }

    gravity_vm_runclosure(vm, closure, VALUE_FROM_NULL, params, numArgs);
}

void ScriptManager::Destroy()
{
    gravity_compiler_free(compiler);
    gravity_vm_free(vm);
    gravity_core_free();
}

void ScriptManager::ErrorCallback(gravity_vm *vm, error_type_t error_type, const char *message, error_desc_t error_desc, void *xdata)
{
    printf("Gravity error: %s", message);
}
