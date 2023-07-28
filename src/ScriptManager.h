// RetroSim - Copyright 2011-2023 Zoltán Majoros. All rights reserved.
// https://github.com/arcanelab

#include <string>
#include "gravity_vm.h"
#include "gravity_delegate.h"
#include "gravity_compiler.h"
#include "gravity_macros.h"
#include "gravity_core.h"
#include <vector>

class ScriptManager
{
public:
    void Update(float deltaTime);
    void Destroy();
    void CompileScript(std::string script);
    void RunScript(std::string functionName, std::vector<gravity_value_t> args, const int numArgs);
    // ScriptManager() {}
    // ~ScriptManager()
    // {
    //     Destroy();
    // }

private:
    static void ErrorCallback(gravity_vm *vm, error_type_t error_type, const char *message, error_desc_t error_desc, void *xdata);

    std::string script;

    gravity_delegate_t delegate = {.error_callback = ErrorCallback};
    gravity_compiler_t *compiler = gravity_compiler_create(&delegate);
    gravity_vm *vm = gravity_vm_new(&delegate);
    gravity_closure_t *closure;
};
