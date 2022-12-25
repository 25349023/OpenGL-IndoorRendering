#include "DeferredRenderer.h"

#include <GLM/gtc/type_ptr.hpp>

DeferredRenderer::DeferredRenderer(int na, glm::ivec2 ws)
{
    numAttachment = na;
    updateWindowSize(ws);
    glEnable(GL_DEPTH_TEST);
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
    for (int i = 0; i < numAttachment; ++i)
    {
        attachNewFBTexture();
    }
    activeTex = FRAG_COLOR;

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
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F,
        winSize.x, winSize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachment, GL_TEXTURE_2D, tex, 0);
}

void DeferredRenderer::updateWindowSize(glm::ivec2 ws)
{
    winSize = ws;
    glViewport(0, 0, ws.x, ws.y);
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

void DeferredRenderer::rotateCamera(const float rad) {
    glm::mat4 vt = glm::transpose(viewMat);
    glm::vec4 yAxisVec4 = vt[1];
    glm::vec3 yAxis(yAxisVec4.x, yAxisVec4.y, yAxisVec4.z);
    glm::quat q = glm::angleAxis(rad, yAxis);
    glm::mat4 rotMat = glm::toMat4(q);
    glm::vec3 p = camCenter - camEye;
    glm::vec4 resP = rotMat * glm::vec4(p.x, p.y, p.z, 1.0);

    camCenter = glm::vec3(resP.x + camEye.x, resP.y + camEye.y, resP.z + camEye.z);
    recalculateCameraLocals();
}

void DeferredRenderer::recalculateCameraLocals()
{
    camLocalZ = camCenter - camEye;
    camLocalZ.y = 0;
    camLocalZ = glm::normalize(camLocalZ);

    glm::vec3 side = glm::cross(camLocalZ, camUp);
    camLocalY = glm::normalize(glm::cross(side, camLocalZ));

}

void DeferredRenderer::updateViewMat()
{
    viewMat = glm::lookAt(camEye, camCenter, camUp);
}

void DeferredRenderer::translateCamera(glm::vec3 amount)
{
    camEye += amount;
    camCenter += amount;
}

void DeferredRenderer::prepareFirstStage()
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glDrawBuffers(attachedTexs.size(), drawBuffers.data());
    clear();
    fbufSP->useProgram();
    glUniformMatrix4fv((*fbufSP)["projMat"], 1, false, glm::value_ptr(this->projMat));
    glUniformMatrix4fv((*fbufSP)["viewMat"], 1, false, glm::value_ptr(this->viewMat));
}

void DeferredRenderer::secondStage()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    clear();
    screenSP->useProgram();

    glBindVertexArray(frameVao);
    glUniform1i((*screenSP)["activeTex"], activeTex);
    glUniform3fv((*screenSP)["cameraEye"], 1, glm::value_ptr(camEye));
    glUniform3fv((*screenSP)["directionalLight"], 1, glm::value_ptr(dirLight));

    for (int i = 0; i < GBUFFER_COUNT; ++i)
    {
        glUniform1i(i, i);
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, attachedTexs[i]);
    }
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glBindVertexArray(0);
}

void DeferredRenderer::clear()
{
    static const float COLOR[] = { 0.1, 0.1, 0.1, 1.0 };
    static const float DEPTH[] = { 1.0 };

    for (int i = 0; i < attachedTexs.size(); ++i)
    {
        glClearBufferfv(GL_COLOR, i, COLOR);
    }
    glClearBufferfv(GL_DEPTH, 0, DEPTH);
}
