// RetroSim - Copyright 2011-2023 Zoltán Majoros. All rights reserved.
// https://github.com/arcanelab

#include "GravityScripting.h"
#include "FileUtils.h"
#include "gravity_compiler.h"
#include "gravity_macros.h"
#include "gravity_core.h"
#include "GravityAPI.h"
#include <string>
#include <sstream>

namespace RetroSim::GravityScripting
{
    // forward declarations
    std::string GetScriptLine(const std::string &script, uint32_t lineNumber);
    void ErrorCallback(gravity_vm *vm, error_type_t error_type, const char *message, error_desc_t error_desc, void *xdata);

    std::string script;
    ScriptError *lastError = nullptr;

    gravity_delegate_t delegate = {.error_callback = ErrorCallback};
    gravity_compiler_t *compiler = nullptr;
    gravity_vm *vm = nullptr;
    gravity_closure_t *closure;

    void Initialize()
    {
        compiler = gravity_compiler_create(&delegate);
        vm = gravity_vm_new(&delegate);
    }

    void RegisterAPIFunctions()
    {
        GravityAPI::RegisterAPIFunctions(vm);
    }

    // Compiles the script and transfers it to the VM
    bool CompileScript(std::string _script)
    {
        if(lastError != nullptr)
        {
            delete lastError;
            lastError = nullptr;
        }

        script = _script;
        const char *scriptCStr = script.c_str();
        closure = gravity_compiler_run(compiler, scriptCStr, (uint32_t)strlen(scriptCStr), 0, true, true);
        if(closure == NULL || lastError != nullptr)
        {
            return false;
        }
        gravity_compiler_transfer(compiler, vm);
        gravity_vm_loadclosure(vm, closure);
        return true;
    }

    bool CompileScriptFromFile(std::string filename)
    {
        script = RetroSim::ReadTextFile(filename);
        if (script.empty())
            return false;

        return CompileScript(script);
    }

    void RunScript(std::string functionName, std::vector<gravity_value_t> args, const int numArgs)
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

    void Cleanup()
    {
        gravity_compiler_free(compiler);
        compiler = nullptr;
        gravity_vm_reset(vm);
        gravity_vm_free(vm);
        vm = nullptr;
    }

    std::string GetScriptLine(const std::string &script, uint32_t lineNumber)
    {
        std::string line;
        std::istringstream ss(script);
        for (uint32_t i = 0; i < lineNumber; i++)
            std::getline(ss, line);

        return line;
    }

    void ErrorCallback(gravity_vm *vm, error_type_t error_type, const char *message, error_desc_t error_desc, void *xdata)
    {
        if(lastError != nullptr)
        {
            delete lastError;
            lastError = nullptr;
        }

        std::ostringstream oss;
        oss << "Gravity error in line " << error_desc.lineno << ": " << message << std::endl;
        oss << GetScriptLine(script, error_desc.lineno) << std::endl;
        if (error_desc.colno > 0 && error_desc.colno < 100) {
            for (uint32_t i = 0; i < error_desc.colno - 1; i++)
                oss << " ";
            oss << "^" << std::endl;
        }

        lastError = new ScriptError();
        lastError->errorMessage = oss.str();
        lastError->lineNumber = error_desc.colno;
    }
}
