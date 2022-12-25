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
        ImGui::InputFloat3("Camera Eye", glm::value_ptr(camEye));
        ImGui::InputFloat3("Camera LookAt", glm::value_ptr(camCenter));
    }

    if (ImGui::CollapsingHeader("G-Buffers"))
    {
        int* atexPtr = (int*)&deferredRenderer->activeTex;
        auto& atexs = deferredRenderer->attachedTexs;

        ImGui::RadioButton("Render Result", atexPtr, atexs[FRAG_COLOR]);
        ImGui::SameLine();
        ImGui::RadioButton("World Vertex", atexPtr, atexs[WORLD_VERTEX]);
        ImGui::SameLine();
        ImGui::RadioButton("World Normal", atexPtr, atexs[WORLD_NORMAL]);

        ImGui::RadioButton("Ambient Color", atexPtr, atexs[AMBIENT_COLOR]);
        ImGui::SameLine();
        ImGui::RadioButton("Diffuse Color", atexPtr, atexs[DIFFUSE_COLOR]);
        ImGui::SameLine();
        ImGui::RadioButton("Specular Color", atexPtr, atexs[SPECULAR_COLOR]);
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
