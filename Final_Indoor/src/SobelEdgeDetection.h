#pragma once
#include <GLM/glm.hpp>

#include "Shader.h"
#include "glad/glad.h"

class SobelEdgeDetection
{
public:
    SobelEdgeDetection(glm::ivec2 ws, ShaderProgram* sp);

    GLuint renderEdge(GLuint windowVao, GLuint tex);
    void setupFrameBuffer(glm::ivec2 ws);
    void teardownFrameBuffer();
    
private:
    ShaderProgram* edgeSP{};
    
    GLuint altFbo[1]{};
    GLuint altTex[1]{};

};
