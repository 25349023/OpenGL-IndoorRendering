#include "DeferredRenderer.h"

DeferredRenderer::DeferredRenderer(glm::vec2 ws)
{
    winSize = ws;
    setupFrameBuffer();
}

void DeferredRenderer::setupFrameBuffer()
{
    // vao for framebuffer shader
    glGenVertexArrays(1, &frameVao);
    glBindVertexArray(frameVao);

    const float windowVertex[] =
    {
        //vec2 position vec2 texture_coord
        1.0f, -1.0f, 1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f, 0.0f,
        -1.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f
    };

    glGenBuffers(1, &windowVbo);
    glBindBuffer(GL_ARRAY_BUFFER, windowVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(windowVertex), windowVertex, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (void*)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (void*)(sizeof(float) * 2));

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    // setup framebuffer
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // Create fboDataTexture
    attachedTexs.clear();
    drawBuffers.clear();
    attachNewFBTexture();
    activateFBTexture(0);

    // Create Depth RBO
    glGenRenderbuffers(1, &depthRbo);
    glBindRenderbuffer(GL_RENDERBUFFER, depthRbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, winSize.x, winSize.y);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRbo);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindVertexArray(0);
}

void DeferredRenderer::genFBTexture(GLuint& tex, int attachment)
{
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
        winSize.x, winSize.y, 0, GL_RGBA,GL_UNSIGNED_BYTE, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachment, GL_TEXTURE_2D, tex, 0);
}

void DeferredRenderer::updateWindowSize(glm::vec2 ws)
{
    winSize = ws;
    setupFrameBuffer();
}

int DeferredRenderer::attachNewFBTexture()
{
    attachedTexs.push_back(0);
    int idx = attachedTexs.size() - 1;
    genFBTexture(attachedTexs[idx], idx);
    drawBuffers.push_back(GL_COLOR_ATTACHMENT0 + idx);

    return idx;
}

void DeferredRenderer::beforeFirstStage()
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glDrawBuffers(attachedTexs.size(), drawBuffers.data());
    clear();
    fbufShaderProgram->useProgram();
}

void DeferredRenderer::secondStage()
{
    const int fbtexLoc = 0;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    clear();
    screenShaderProgram->useProgram();

    glUniform1i(fbtexLoc, 0);
    glBindVertexArray(frameVao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, activeTex);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glBindVertexArray(0);
}

void DeferredRenderer::clear()
{
    static const float COLOR[] = { 0.1, 0.1, 0.1, 1.0 };
    static const float DEPTH[] = { 1.0 };

    glClearBufferfv(GL_COLOR, 0, COLOR);
    glClearBufferfv(GL_DEPTH, 0, DEPTH);
}

void DeferredRenderer::activateFBTexture(int target)
{
    activeTex = attachedTexs[target];
}
