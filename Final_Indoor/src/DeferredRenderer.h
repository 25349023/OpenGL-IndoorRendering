#pragma once

#include <array>
#include <vector>
#include <glad/glad.h>

#include "DirectionalShadowMapper.h"
#include "Model.h"
#include "Shader.h"
#include "GLM/glm.hpp"
#include "glm/gtx/quaternion.hpp"

enum GBuffer
{
    FRAG_COLOR,
    WORLD_VERTEX,
    WORLD_NORMAL,
    AMBIENT_COLOR,
    DIFFUSE_COLOR,
    SPECULAR_COLOR,
    SHININESS,
    SHADOW_MAP,
    GBUFFER_COUNT
};

enum Feature
{
    BLINN_PHONG_SHADING,
    DIR_SHADOW_MAPPING,
    FEATURE_COUNT
};

class DeferredRenderer
{
public:
    DeferredRenderer(glm::ivec2 ws);

    // [TODO] free the old source when resizing the window
    void updateWindowSize(glm::ivec2 ws);
    void appendSceneObj(Model* model);

    void translateCamera(glm::vec3 amount);
    void rotateCamera(const float rad);
    void recalculateCameraLocals();
    void updateViewMat();

    void shadowMapStage();
    void firstStage();
    void secondStage();

    void clear();

    friend class MyImGuiPanel;

    glm::vec3 camEye{ 4.0, 1.0, -1.5 };
    glm::vec3 camCenter{ 3.0, 1.0, -1.5 };
    glm::vec3 camUp{ 0.0, 1.0, 0.0 };
    glm::vec3 camLocalY{ 0, 1, 0 };
    glm::vec3 camLocalZ{ 0, 0, -1 };

    // [TODO] make fbufSP, screenSP private and initialize through the ctor
    ShaderProgram* fbufSP{};
    ShaderProgram* screenSP{};

    glm::mat4 projMat{ 1.0 };
    glm::mat4 viewMat{ 1.0 };

    DirectionalShadowMapper* dirShadowMapper{};

private:
    GLuint frameVao{};
    GLuint windowVbo{};
    GLuint fbo{};
    GLuint depthRbo{};

    std::vector<Model*> sceneObjects;

    std::vector<GLuint> attachedTexs{};
    std::vector<GLenum> drawBuffers{};
    GLuint activeTex{};

    std::array<bool, FEATURE_COUNT> enableFeature{};

    glm::ivec2 winSize{};

    int attachNewFBTexture();
    void setupFrameBuffer();
    void genFBTexture(GLuint& tex, int attachment);
};
