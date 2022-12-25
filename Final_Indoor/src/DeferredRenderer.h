#pragma once

#include <vector>
#include <glad/glad.h>

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

class DeferredRenderer
{
public:
    DeferredRenderer(int na, glm::vec2 ws);

    void updateWindowSize(glm::vec2 ws);
    int attachNewFBTexture();

    void beforeFirstStage();
    void secondStage();

    void clear();

    void setFbufShaderProgram(ShaderProgram* fbuf_shader_program);
    void setScreenShaderProgram(ShaderProgram* screen_shader_program);

    friend class MyImGuiPanel;
private:
    ShaderProgram* fbufShaderProgram{};
    ShaderProgram* screenShaderProgram{};

    GLuint frameVao{};
    GLuint windowVbo{};
    GLuint fbo{};
    GLuint depthRbo{};

    int numAttachment;

    std::vector<GLuint> attachedTexs{};
    std::vector<GLenum> drawBuffers{};
    GLuint activeTex{};
    glm::vec3 dirLight{ -2.845, 2.028, -1.293 };

    GLuint activeTexHandle;
    GLuint directionalLightHandle;

    glm::ivec2 winSize{};

    void setupFrameBuffer();

    void genFBTexture(GLuint& tex, int attachment);
};
