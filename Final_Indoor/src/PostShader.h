#pragma once
#include <GLM/glm.hpp>

#include "Shader.h"
#include "glad/glad.h"

class PostShader
{
public:
    PostShader(glm::ivec2 ws, ShaderProgram* sp);

    GLuint render(GLuint windowVao, GLuint tex);
    void setupFrameBuffer(glm::ivec2 ws);
    void teardownFrameBuffer();
    
private:
    ShaderProgram* SP{};
    
    GLuint Fbo[1]{};
    GLuint Tex[1]{};

};
