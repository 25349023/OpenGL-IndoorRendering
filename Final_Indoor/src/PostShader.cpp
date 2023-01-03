#include "PostShader.h"

#include <iostream>


PostShader::PostShader(glm::ivec2 ws, ShaderProgram* sp) : SP(sp)
{
    setupFrameBuffer(ws);
}

void PostShader::setupFrameBuffer(glm::ivec2 ws)
{
    glGenFramebuffers(1, Fbo);
    glGenTextures(1, Tex);

    glBindFramebuffer(GL_FRAMEBUFFER, Fbo[0]);
    glBindTexture(GL_TEXTURE_2D, Tex[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, ws.x, ws.y, 0, GL_RGBA, GL_FLOAT, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, Tex[0], 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "Error binding frame buffer: " << std::hex <<
                glCheckFramebufferStatus(GL_FRAMEBUFFER) << std::endl;
    }
    
}

void PostShader::teardownFrameBuffer()
{
    glDeleteFramebuffers(1, Fbo);
    glDeleteTextures(1, Tex);
}

GLuint PostShader::render(GLuint windowVao, GLuint tex)
{
    SP->useProgram();
    glUniform1i((*SP)["tex"], 0);
    
    glBindFramebuffer(GL_FRAMEBUFFER, Fbo[0]);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);
        
    glBindVertexArray(windowVao);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glBindVertexArray(0);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return Tex[0];
}
