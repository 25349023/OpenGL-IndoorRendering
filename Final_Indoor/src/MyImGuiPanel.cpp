#include "MyImGuiPanel.h"

#include <GLM/gtc/type_ptr.hpp>


MyImGuiPanel::MyImGuiPanel()
{
    this->m_avgFPS = 0.0;
    this->m_avgFrameTime = 0.0;
}


MyImGuiPanel::~MyImGuiPanel() {}

void MyImGuiPanel::update()
{
    // performance information
    const std::string FPS_STR = "FPS: " + std::to_string(this->m_avgFPS);
    ImGui::TextColored(ImVec4(0, 180, 0, 210), FPS_STR.c_str());
    const std::string FT_STR = "Frame: " + std::to_string(this->m_avgFrameTime);
    ImGui::TextColored(ImVec4(0, 180, 0, 210), FT_STR.c_str());

    if (ImGui::CollapsingHeader("Camera settings"))
    {
        ImGui::InputFloat3("Camera Eye", glm::value_ptr(deferredRenderer->camEye));
        ImGui::InputFloat3("Camera LookAt", glm::value_ptr(deferredRenderer->camCenter));
    }

    if (ImGui::CollapsingHeader("G-Buffers"))
    {
        int* atexPtr = (int*)&deferredRenderer->activeTex;

        ImGui::RadioButton("Render Result", atexPtr, FRAG_COLOR);
        ImGui::SameLine();
        ImGui::RadioButton("World Vertex", atexPtr, WORLD_VERTEX);
        ImGui::SameLine();
        ImGui::RadioButton("World Normal", atexPtr, WORLD_NORMAL);

        ImGui::RadioButton("Ambient Color", atexPtr, AMBIENT_COLOR);
        ImGui::SameLine();
        ImGui::RadioButton("Diffuse Color", atexPtr, DIFFUSE_COLOR);
        ImGui::SameLine();
        ImGui::RadioButton("Specular Color", atexPtr, SPECULAR_COLOR);
        ImGui::RadioButton("Shadow Map", atexPtr, SHADOW_MAP);
    }

    auto enable = deferredRenderer->enableFeature.data();
    if (ImGui::CollapsingHeader("Blinn-Phong Shading"))
    {
        ImGui::PushID(BLINN_PHONG_SHADING);
        ImGui::Checkbox("Enable", enable + BLINN_PHONG_SHADING);
        ImGui::PopID();
        ImGui::DragFloat3("Light Position", glm::value_ptr(deferredRenderer->nearDirLight), 0.2);
    }
    if (ImGui::CollapsingHeader("Directional Shadow Mapping"))
    {
        ImGui::PushID(DIR_SHADOW_MAPPING);
        ImGui::Checkbox("Enable", enable + DIR_SHADOW_MAPPING);
        ImGui::PopID();
    }
}

void MyImGuiPanel::setAvgFPS(const double avgFPS)
{
    this->m_avgFPS = avgFPS;
}

void MyImGuiPanel::setAvgFrameTime(const double avgFrameTime)
{
    this->m_avgFrameTime = avgFrameTime;
}
