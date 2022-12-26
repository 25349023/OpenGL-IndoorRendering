#pragma once

#include "Shader.h"
#include "Model.h"
#include "glad/glad.h"
#include "GLM/glm.hpp"
#include "glm/gtx/quaternion.hpp"


class DirectionalShadowMapper
{
public:
    glm::vec3 lightEye{ -2.845, 2.028, -1.293 };
    glm::vec3 lightLookAt{ 0.542, -0.141, -0.422 };

    DirectionalShadowMapper(ShaderProgram* sp);

    void setupDepthFrameBuffer();
    void updateLightVPMat();

    void beforeRender();
    void renderShadowMap(const std::vector<Model*>& sceneObjs);
    glm::mat4 getShadowSBPVMat();

    friend class DeferredRenderer;
private:
    GLuint depthFbo{};
    GLuint depthTex{};
    ShaderProgram* depthSP{};
    int shadowMapRes{ 1024 };

    glm::mat4 lightViewMat{ 1.0 };
    glm::mat4 lightProjMat{ 1.0 };
};
