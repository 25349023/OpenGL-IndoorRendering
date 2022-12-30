#include "SobelEdgeDetection.h"

#include <iostream>


SobelEdgeDetection::SobelEdgeDetection(glm::ivec2 ws, ShaderProgram* sp) : edgeSP(sp)
{
    setupFrameBuffer(ws);
}

void SobelEdgeDetection::setupFrameBuffer(glm::ivec2 ws)
{
    glGenFramebuffers(1, altFbo);
    glGenTextures(1, altTex);

    glBindFramebuffer(GL_FRAMEBUFFER, altFbo[0]);
    glBindTexture(GL_TEXTURE_2D, altTex[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, ws.x, ws.y, 0, GL_RGBA, GL_FLOAT, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, altTex[0], 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "Error binding frame buffer: " << std::hex <<
                glCheckFramebufferStatus(GL_FRAMEBUFFER) << std::endl;
    }
    
}

void SobelEdgeDetection::teardownFrameBuffer()
{
    glDeleteFramebuffers(1, altFbo);
    glDeleteTextures(1, altTex);
}

GLuint SobelEdgeDetection::renderEdge(GLuint windowVao, GLuint tex)
{
    edgeSP->useProgram();
    glUniform1i((*edgeSP)["tex"], 0);
    
    glBindFramebuffer(GL_FRAMEBUFFER, altFbo[0]);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);
        
    glBindVertexArray(windowVao);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glBindVertexArray(0);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return altTex[0];
}
