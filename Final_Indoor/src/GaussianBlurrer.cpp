#include "GaussianBlurrer.h"

#include <iostream>


GaussianBlurrer::GaussianBlurrer(glm::ivec2 ws, ShaderProgram* sp) : blurSP(sp) 
{
    setupFrameBuffer(ws);
}

void GaussianBlurrer::setupFrameBuffer(glm::ivec2 ws)
{
    glGenFramebuffers(2, altFbo);
    glGenTextures(2, altTex);

    for (int i = 0; i < 2; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, altFbo[i]);
        glBindTexture(GL_TEXTURE_2D, altTex[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, ws.x, ws.y, 0, GL_RGBA, GL_FLOAT, NULL);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, altTex[i], 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            std::cout << "Error binding frame buffer: " << std::hex <<
                    glCheckFramebufferStatus(GL_FRAMEBUFFER) << std::endl;
        }
    }
}

void GaussianBlurrer::teardownFrameBuffer()
{
    glDeleteFramebuffers(2, altFbo);
    glDeleteTextures(2, altTex);
}

GLuint GaussianBlurrer::renderBlur(GLuint windowVao, GLuint initEmission)
{
    bool horizontal = true, first_iteration = true;
    const int amount = 6;
    blurSP->useProgram();
    glUniform1i((*blurSP)["tex"], 0);
    
    for (int i = 0; i < amount; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, altFbo[horizontal]);
        glUniform1i((*blurSP)["horizontal"], horizontal);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, first_iteration ? initEmission : altTex[!horizontal]); 
        
        glBindVertexArray(windowVao);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        glBindVertexArray(0);
        
        horizontal = !horizontal;
        if (first_iteration)
            first_iteration = false;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return altTex[!horizontal];
}
