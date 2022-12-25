#pragma once

#include <vector>
#include <glad/glad.h>

#include "RenderSetting.h"
#include "Shader.h"
#include "GLM/glm.hpp"

enum GBuffer
{
    FRAG_COLOR,
    WORLD_VERTEX,
    WORLD_NORMAL,
    AMBIENT_COLOR,
    DIFFUSE_COLOR,
    SPECULAR_COLOR,
    SHININESS,
    GBUFFER_COUNT
};

extern RenderSetting* renderSetting;

class DeferredRenderer
{
public:
    DeferredRenderer(int na, glm::vec2 ws);

    void updateWindowSize(glm::vec2 ws);
    int attachNewFBTexture();

    void beforeFirstStage();
    void secondStage();

    void clear();

    friend class MyImGuiPanel;

    glm::vec3 camEye{ 4.6, 1.2, -2.0 };
    glm::vec3 camCenter{ 4.5, 1.2, -2.0 };

    ShaderProgram* fbufSP{};
    ShaderProgram* screenSP{};
private:

    GLuint frameVao{};
    GLuint windowVbo{};
    GLuint fbo{};
    GLuint depthRbo{};

    int numAttachment;

    std::vector<GLuint> attachedTexs{};
    std::vector<GLenum> drawBuffers{};
    GLuint activeTex{};
    glm::vec3 dirLight{ -2.845, 2.028, -1.293 };

    glm::ivec2 winSize{};

    void setupFrameBuffer();

    void genFBTexture(GLuint& tex, int attachment);
};
