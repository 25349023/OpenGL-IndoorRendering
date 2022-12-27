#pragma once
#include <GLM/glm.hpp>

#include "Shader.h"
#include "glad/glad.h"

class GaussianBlurrer
{
public:
    GaussianBlurrer(glm::ivec2 ws, ShaderProgram* sp);

    GLuint renderBlur(GLuint windowVao, GLuint initEmission);
    void setupFrameBuffer(glm::ivec2 ws);
    
private:
    ShaderProgram* blurSP{};
    
    GLuint altFbo[2]{};
    GLuint altTex[2]{};

};
