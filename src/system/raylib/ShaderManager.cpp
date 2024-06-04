#include <string>
#include "ShaderManager.h"
#include "FileUtils.h"
#include "GPU.h"
#include "Core.h"
#include "imgui.h"

namespace RetroSim::Raylib
{
    void ShaderManager::SetupShaders(RetroSim::Core *core, const int scaledWindowWidth, const int scaledWindowHeight)
    {
        std::string vertexShaderFileName = "lottes.vs";
        std::string fragmentShaderFileName = "lottes.fs";

        std::string dataPath = core->GetCoreConfig().GetDataPath();
        std::string vertexShaderPath = ConvertPathToPlatformCompatibleFormat(dataPath + "/shaders/" + vertexShaderFileName);
        std::string fragmentShaderPath = ConvertPathToPlatformCompatibleFormat(dataPath + "/shaders/" + fragmentShaderFileName);

        LogPrintf(RETRO_LOG_INFO, "Data path: %s\n", dataPath.c_str());
        LogPrintf(RETRO_LOG_INFO, "Fragment shader path: %s\n", fragmentShaderPath.c_str());
        LogPrintf(RETRO_LOG_INFO, "Vertex shader path: %s\n", vertexShaderPath.c_str());

        Shader loadedShader = LoadShader(vertexShaderPath.c_str(), fragmentShaderPath.c_str());
        this->shader = loadedShader;

        Vector2 textureSize = {(float)GPU::textureWidth, (float)GPU::textureHeight};
        Vector2 outputSize = {(float)scaledWindowWidth, (float)scaledWindowHeight};
        Vector2 inputSize = {(float)scaledWindowWidth, (float)scaledWindowHeight};

        // Set shader values
        SetShaderValue(shader, GetShaderLocation(shader, "TextureSize"), &textureSize, SHADER_UNIFORM_VEC2);
        SetShaderValue(shader, GetShaderLocation(shader, "InputSize"), &textureSize, SHADER_UNIFORM_VEC2);
        SetShaderValue(shader, GetShaderLocation(shader, "OutputSize"), &outputSize, SHADER_UNIFORM_VEC2);
        UpdateShaderVariables();
    }

    void ShaderManager::UpdateShaderVariables()
    {
        SetShaderValue(shader, GetShaderLocation(shader, "hardScan"), &parameters.hardScan, SHADER_ATTRIB_FLOAT);
        SetShaderValue(shader, GetShaderLocation(shader, "hardPix"), &parameters.hardPix, SHADER_ATTRIB_FLOAT);
        SetShaderValue(shader, GetShaderLocation(shader, "warpX"), &parameters.warpX, SHADER_ATTRIB_FLOAT);
        SetShaderValue(shader, GetShaderLocation(shader, "warpY"), &parameters.warpY, SHADER_ATTRIB_FLOAT);
        SetShaderValue(shader, GetShaderLocation(shader, "maskDark"), &parameters.maskDark, SHADER_ATTRIB_FLOAT);
        SetShaderValue(shader, GetShaderLocation(shader, "maskLight"), &parameters.maskLight, SHADER_ATTRIB_FLOAT);
        SetShaderValue(shader, GetShaderLocation(shader, "scaleInLinearGamma"), &parameters.scaleInLinearGamma, SHADER_ATTRIB_FLOAT);
        SetShaderValue(shader, GetShaderLocation(shader, "shadowMask"), &parameters.shadowMask, SHADER_ATTRIB_FLOAT);
        SetShaderValue(shader, GetShaderLocation(shader, "brightBoost"), &parameters.brightBoost, SHADER_ATTRIB_FLOAT);
        SetShaderValue(shader, GetShaderLocation(shader, "hardBloomPix"), &parameters.hardBloomPix, SHADER_ATTRIB_FLOAT);
        SetShaderValue(shader, GetShaderLocation(shader, "hardBloomScan"), &parameters.hardBloomScan, SHADER_ATTRIB_FLOAT);
        SetShaderValue(shader, GetShaderLocation(shader, "bloomAmount"), &parameters.bloomAmount, SHADER_ATTRIB_FLOAT);
        SetShaderValue(shader, GetShaderLocation(shader, "shape"), &parameters.shape, SHADER_ATTRIB_FLOAT);
    }

    void ShaderManager::DrawParametersGui()
    {
        ImGui::Begin("Shader Parameters");
        ImGui::SliderFloat("hardScan", &parameters.hardScan, -20.0f, 0.0f, "%.1f");
        ImGui::SliderFloat("hardPix", &parameters.hardPix, -20.0f, 0.0f, "%.1f");
        ImGui::SliderFloat("warpX", &parameters.warpX, 0.0f, 0.125f, "%.3f", ImGuiSliderFlags_Logarithmic);
        ImGui::SliderFloat("warpY", &parameters.warpY, 0.0f, 0.125f, "%.3f", ImGuiSliderFlags_Logarithmic);
        ImGui::SliderFloat("maskDark", &parameters.maskDark, 0.0f, 2.0f, "%.1f");
        ImGui::SliderFloat("maskLight", &parameters.maskLight, 0.0f, 2.0f, "%.1f");
        ImGui::SliderFloat("scaleInLinearGamma", &parameters.scaleInLinearGamma, 0.0f, 1.0f, "%.1f");
        ImGui::SliderFloat("shadowMask", &parameters.shadowMask, 0.0f, 4.0f, "%1.0f");
        ImGui::SliderFloat("brightness boost", &parameters.brightBoost, 0.0f, 2.0f, "%.2f");
        ImGui::SliderFloat("bloom-x soft", &parameters.hardBloomPix, -2.0f, -0.5f, "%.1f");
        ImGui::SliderFloat("bloom-y soft", &parameters.hardBloomScan, -4.0f, -1.0f, "%.1f");
        ImGui::SliderFloat("bloom ammount", &parameters.bloomAmount, 0.0f, 1.0f, "%.2f");
        ImGui::SliderFloat("filter kernel shape", &parameters.shape, 0.0f, 10.0f, "%.2f");

        ImGui::End();
    }
}
