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
        float f[3] = {0};
        ImGui::InputFloat3("Camera Eye", glm::value_ptr(camEye));
        ImGui::InputFloat3("Camera LookAt", glm::value_ptr(camCenter));
        // ImGui::TreePop();
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
