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
}

void ScriptManager::RunScript(std::string functionName, std::vector<gravity_value_t> args, const int numArgs)
{
    gravity_vm_loadclosure(vm, closure);
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

// bool ScriptManager::Initialize()
// {
//     const char *source_code = R"(
//         func main()
//         {
//             System.print("Hello world from Gravity!")
//             return 42
//         }

//         func update(dt)
//         {
//             System.print(dt)
//         }

//         )";

//     gravity_delegate_t delegate = {.error_callback = ErrorCallback};
//     gravity_compiler_t *compiler = gravity_compiler_create(&delegate);
//     gravity_closure_t *closure = gravity_compiler_run(compiler, source_code, (uint32_t)strlen(source_code), 0, true, true);
//     gravity_vm *vm = gravity_vm_new(&delegate);
//     gravity_compiler_transfer(compiler, vm);
//     gravity_compiler_free(compiler);

//     if (gravity_vm_runmain(vm, closure))
//     {
//         gravity_value_t result = gravity_vm_result(vm);

//         char buffer[512];
//         gravity_value_dump(vm, result, buffer, sizeof(buffer));
//         printf("%s\n", buffer);
//     }

//     gravity_vm_loadclosure(vm, closure);
//     gravity_value_t updateFunction = gravity_vm_getvalue(vm, "update", strlen("update"));

//     gravity_closure_t *updateClosure = VALUE_AS_CLOSURE(updateFunction);
//     gravity_value_t deltaTime = VALUE_FROM_FLOAT(rand() / (float)RAND_MAX);
//     gravity_value_t params[] = {deltaTime};

//     gravity_vm_runclosure(vm, updateClosure, VALUE_FROM_NULL, params, 1);

//     gravity_vm_free(vm);
//     gravity_core_free();

//     return false;
// }
