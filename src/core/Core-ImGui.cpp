#ifdef IMGUI
#include "Core.h"
#include "imgui.h"
#include "imfilebrowser.h"
#include "GravityScripting.h"

bool showSystem = false;
bool showInfo = false;
bool showConfig = false;
bool showScripts = false;
bool showImGuiDemo = false;

void Core::DrawImGui(bool &showShaderParameters)
{
    if (showSystem)
    {
        ImGui::Begin("System", &showSystem);
        if (ImGui::Button("Reset system"))
        {
            Reset();
        }

        ImGui::SameLine();

        if (ImGui::Button("Quit system"))
        {
            std::exit(0);
        }

        ImGui::Checkbox("Paused", &isPaused);
        ImGui::Checkbox("Scripting Enabled", &scriptingEnabled);

        if (ImGui::Button("Reset CPU"))
        {
            cpu.Reset();
        }

        if (isPaused)
        {
            if (ImGui::Button("Run next frame"))
            {
                RunNextFrame();
            }

            if (ImGui::Button("Step CPU"))
            {
                cpu.sleep = false;
                cpu.Tick();
            }
        }
        ImGui::End();
    }

    if (showInfo)
    {
        ImGui::Begin("Info", &showInfo);
        ImGuiTreeNodeFlags openHeader = ImGuiTreeNodeFlags_DefaultOpen;

        if (ImGui::CollapsingHeader("System", openHeader))
        {
            ImGui::Text("Fixed Frame Time: %u µs", MMU::memory.generalRegisters.fixedFrameTime);
            ImGui::Text("Delta Time: %u µs", MMU::memory.generalRegisters.deltaTime);
            ImGui::Text("Frame Counter: %u", MMU::memory.generalRegisters.frameCounter);
            ImGui::Text("Refresh Rate: %d Hz", MMU::memory.generalRegisters.refreshRate);
            ImGui::Text("Current FPS: %d Hz", MMU::memory.generalRegisters.currentFPS);
        }

        if (ImGui::CollapsingHeader("GPU", openHeader))
        {
            ImGui::Text("Screen Width: %d", MMU::memory.gpu.screenWidth);
            ImGui::Text("Screen Height: %d", MMU::memory.gpu.screenHeight);
            ImGui::Text("Tile Width: %d", MMU::memory.gpu.tileWidth);
            ImGui::Text("Tile Height: %d", MMU::memory.gpu.tileHeight);
            ImGui::Text("Map Width: %d", MMU::memory.gpu.mapWidth);
            ImGui::Text("Map Height: %d", MMU::memory.gpu.mapHeight);
            ImGui::Text("Sprite Atlas Pitch: %d", MMU::memory.gpu.spriteAtlasPitch);
        }

        if (ImGui::CollapsingHeader("A65000 CPU", openHeader))
        {
            ImGui::Checkbox("Sleep", &cpu.sleep);
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "General registers:");

            const int num_columns = 4;
            if (ImGui::BeginTable("Registers", num_columns))
            {
                for (int i = 0; i < 16; i++)
                {
                    if (i % 4 == 0)
                    {
                        ImGui::TableNextRow();
                    }

                    ImGui::TableSetColumnIndex(i % 4);

                    if (i < 14)
                        ImGui::Text("R%d: %X", i, cpu.registers[i]);
                    else if (i == 14)
                        ImGui::Text("SP: %X", cpu.registers[14]);
                    else
                        ImGui::Text("PC: %X", cpu.registers[15]);
                }

                ImGui::EndTable();
            }

            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Status register:");
            ImGui::Text("Z: %X  N: %X  C: %X  V: %X  B: %X  I: %X",
                        cpu.statusRegister.z, cpu.statusRegister.n, cpu.statusRegister.c,
                        cpu.statusRegister.v, cpu.statusRegister.b, cpu.statusRegister.i);
        }
        ImGui::End();
    }

    if (showConfig)
    {
        ImGui::Begin("Config", &showConfig);

        ImGui::Text("Data Path: %s", coreConfig.GetDataPath().c_str());
        ImGui::Text("Script Path: %s", coreConfig.GetScriptPath().c_str());
        ImGui::Text("Fullscreen: %s", coreConfig.IsFullScreen() ? "True" : "False");
        ImGui::Text("FPS Override: %d", coreConfig.GetFPS());
        ImGui::Text("Audio Sample Rate: %d Hz", coreConfig.GetAudioSampleRate());
        ImGui::Text("Window Scale: %d", coreConfig.GetWindowScale());
        ImGui::InputInt("CPU cycles per frame", &coreConfig.cpuCyclesPerFrame);

        ImGui::End();
    }

    if (showScripts)
    {
        ImGui::Begin("Scripts", &showScripts);

        static ImGui::FileBrowser fileDialog(0, "..");

        // (optional) set browser properties
        fileDialog.SetTitle("Select a script file");
        fileDialog.SetTypeFilters({".gravity"});

        // open file dialog when user clicks this button
        if (ImGui::Button("Load script"))
            fileDialog.Open();

        fileDialog.Display();

        static bool showErrorPopup = false;

        if (fileDialog.HasSelected())
        {
            auto fileName = fileDialog.GetSelected().string();
            std::cout << "Selected filename" << fileName << std::endl;
            fileDialog.ClearSelected();

            scriptingEnabled = false;
            GravityScripting::Cleanup();
            GravityScripting::Initialize();
            GravityScripting::RegisterAPIFunctions();
            scriptingEnabled = GravityScripting::CompileScriptFromFile(fileName);
            if (scriptingEnabled)
            {
                GravityScripting::RunScript("start", {}, 0);
                showErrorPopup = false;
            }
            else
            {
                showErrorPopup = true;
            }
        }

        if (showErrorPopup)
        {
            ImGui::OpenPopup("Script compilation error");

            // Define the popup
            if (ImGui::BeginPopupModal("Script compilation error", NULL, ImGuiWindowFlags_AlwaysAutoResize))
            {
                ImGui::Text(GravityScripting::lastError->errorMessage.c_str());
                if (ImGui::Button("OK"))
                {
                    ImGui::CloseCurrentPopup();
                    showErrorPopup = false;
                }
                ImGui::EndPopup();
            }
        }
        ImGui::End();
    }

    const char *main_menu_items[] =
        {
            "System",
            "Info",
            "Config",
            "Scripts",
            "Shader Parameters",
            "Dear ImGui Demo",
        };

    if (ImGui::IsMouseClicked(1))
    {
        ImGui::OpenPopup("Context Menu");
    }

    if (ImGui::BeginPopupContextWindow("Context Menu"))
    {
        ImGui::MenuItem(main_menu_items[0], NULL, &showSystem);
        ImGui::MenuItem(main_menu_items[1], NULL, &showInfo);
        ImGui::MenuItem(main_menu_items[2], NULL, &showConfig);
        ImGui::MenuItem(main_menu_items[3], NULL, &showScripts);
        ImGui::MenuItem(main_menu_items[4], NULL, &showShaderParameters);
        ImGui::MenuItem(main_menu_items[5], NULL, &showImGuiDemo);

        ImGui::EndPopup();
    }

    if(showImGuiDemo)
    {
        ImGui::ShowDemoWindow(&showImGuiDemo);
    }
}

#endif // IMGUI
