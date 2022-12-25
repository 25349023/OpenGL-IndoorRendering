#pragma once

#include <vector>
#include <glad/glad.h>

#include "Shader.h"
#include "GLM/glm.hpp"

enum GBuffer
{
    FRAG_COLOR, WORLD_VERTEX, WORLD_NORMAL, AMBIENT_COLOR, DIFFUSE_COLOR, SPECULAR_COLOR, GBUFFER_COUNT
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

    ShaderProgram* fbufShaderProgram{};
    ShaderProgram* screenShaderProgram{};

    friend class MyImGuiPanel;
private:
    
    GLuint frameVao{};
    GLuint windowVbo{};
    GLuint fbo{};
    GLuint depthRbo{};

    int numAttachment;
    
    std::vector<GLuint> attachedTexs{};
    std::vector<GLenum> drawBuffers{};
    GLuint activeTex{};
    
    glm::ivec2 winSize{};
    
    void setupFrameBuffer();

    void genFBTexture(GLuint& tex, int attachment);
    void activateFBTexture(GBuffer target);
};
