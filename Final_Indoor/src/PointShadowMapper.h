#pragma once

#include "Shader.h"
#include "Model.h"
#include "glad/glad.h"
#include "GLM/glm.hpp"
#include "glm/gtx/quaternion.hpp"


class PointShadowMapper
{
public:
    glm::vec3 lightPos{ 1.87659, 0.4625, 0.103928 };
    glm::vec3 lightAttenuation{ 1.0, 0.7, 0.14 };
    glm::vec3 lightColor{ 1.0, 1.0, 1.0 };

    PointShadowMapper(ShaderProgram* sp);

    void setupDepthFrameBuffer();
    void updateLightVPMat();

    void beforeRender();
    void renderShadowMap(const std::vector<Model*>& sceneObjs);

    friend class DeferredRenderer;

private:
    GLuint depthFbo{};
    GLuint depthTex{};
    ShaderProgram* depthSP{};
    int shadowMapRes{ 1024 };

    glm::mat4 lightProjMat{ 1.0 };
    std::vector<glm::mat4> lightPVMats{};
};
