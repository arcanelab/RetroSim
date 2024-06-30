#pragma once

namespace RetroSim
{
    class Core;

    class CoreImGui
    {
    public:
        CoreImGui(Core *coreInstance): core(coreInstance) {};

        void DrawImGui(bool &showShaderParameters);
    private:
        Core *core;
    };
};
