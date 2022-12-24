#pragma once

#include <vector>
#include <glad/glad.h>

#include "Shader.h"
#include "GLM/glm.hpp"

enum Attachments
{
    DIFFUSE_COLOR, WORLD_VERTEX, WORLD_NORMAL
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
    void activateFBTexture(Attachments target);
};
