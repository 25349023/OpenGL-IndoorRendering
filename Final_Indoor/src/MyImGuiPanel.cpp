#include "MyImGuiPanel.h"

#include <GLM/gtc/type_ptr.hpp>


MyImGuiPanel::MyImGuiPanel() {}

MyImGuiPanel::~MyImGuiPanel() {}

void MyImGuiPanel::update()
{
    // performance information
    ImGui::TextColored(ImVec4(0, 180, 0, 210), "FPS: %.2f", ImGui::GetIO().Framerate);

    if (ImGui::CollapsingHeader("Camera settings"))
    {
        ImGui::InputFloat3("Camera Eye", glm::value_ptr(deferredRenderer->camEye));
        ImGui::InputFloat3("Camera LookAt", glm::value_ptr(deferredRenderer->camCenter));
    }

    if (ImGui::CollapsingHeader("G-Buffers"))
    {
        int* atexPtr = (int*)&deferredRenderer->activeTex;

        ImGui::RadioButton("Render Result", atexPtr, RENDER_RESULT);
        ImGui::SameLine();
        ImGui::RadioButton("World Vertex", atexPtr, WORLD_VERTEX);
        ImGui::SameLine();
        ImGui::RadioButton("World Normal", atexPtr, WORLD_NORMAL);

        ImGui::RadioButton("Ambient Color", atexPtr, AMBIENT_COLOR);
        ImGui::SameLine();
        ImGui::RadioButton("Diffuse Color", atexPtr, DIFFUSE_COLOR);
        ImGui::SameLine();
        ImGui::RadioButton("Specular Color", atexPtr, SPECULAR_COLOR);
        
        ImGui::RadioButton("Emission Map", atexPtr, EMISSION_MAP);
    }

    auto enable = deferredRenderer->enableFeature.data();
    if (ImGui::CollapsingHeader("Blinn-Phong Shading"))
    {
        ImGui::PushID("Directional");
        ImGui::DragFloat3("Light Position",
            glm::value_ptr(deferredRenderer->dirShadowMapper->lightEye), 0.1f);
        ImGui::DragFloat3("Light LookAt",
            glm::value_ptr(deferredRenderer->dirShadowMapper->lightLookAt), 0.1f);
        ImGui::Checkbox("Enable Lighting", enable + BLINN_PHONG_SHADING);
        ImGui::Checkbox("Enable Directional Shadow Mapping", enable + DIR_SHADOW_MAPPING);
        ImGui::PopID();
    }
    
    ImGui::Checkbox("Enable Normal Mapping", enable + NORMAL_MAPPING);
    
    if (ImGui::CollapsingHeader("Point Light"))
    {
        ImGui::PushID("Point");
        ImGui::DragFloat3("Light Position",
            glm::value_ptr(deferredRenderer->pointLightPos), 0.1f);
        ImGui::DragFloat3("Attenuation Setting",
            glm::value_ptr(deferredRenderer->pointLightAttenuation), 0.1f);
        ImGui::ColorEdit3("Light Color", glm::value_ptr(deferredRenderer->pointLightColor));
        ImGui::Checkbox("Enable Point Light", enable + POINT_LIGHT);
        ImGui::Checkbox("Enable Bloom Effect", enable + BLOOM_EFFECT);
        ImGui::PopID();
    }

}
