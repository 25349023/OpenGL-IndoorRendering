#include "DirectionalShadowMapper.h"

#include <iostream>

DirectionalShadowMapper::DirectionalShadowMapper(ShaderProgram* sp)
{
    depthSP = sp;
    setupDepthFrameBuffer();
    updateLightVPMat();
}

void DirectionalShadowMapper::setupDepthFrameBuffer()
{
    glGenFramebuffers(1, &depthFbo);
    glBindFramebuffer(GL_FRAMEBUFFER, depthFbo);

    glGenTextures(1, &depthTex);
    glBindTexture(GL_TEXTURE_2D, depthTex);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F,
        shadowMapRes, shadowMapRes, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTex, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "Error binding depth frame buffer: " << std::hex <<
                glCheckFramebufferStatus(GL_FRAMEBUFFER) << std::endl;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void DirectionalShadowMapper::updateLightVPMat()
{
    lightViewMat = glm::lookAt(lightEye, lightLookAt, glm::vec3(0.0, 1.0, 0.0));
    lightProjMat = glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, 0.1f, 10.0f);
}

void DirectionalShadowMapper::beforeRender()
{
    glBindFramebuffer(GL_FRAMEBUFFER, depthFbo);
    glDrawBuffer(GL_NONE);
    glViewport(0, 0, shadowMapRes, shadowMapRes);
    depthSP->useProgram();

    updateLightVPMat();
}

void DirectionalShadowMapper::renderShadowMap(const std::vector<Model*>& sceneObjs)
{
    glm::mat4 vpMat = lightProjMat * lightViewMat;

    for (auto model : sceneObjs)
    {
        glm::mat4 modelMat = model->getModelMat().first;
        glm::mat4 mvp = vpMat * modelMat;

        glUniformMatrix4fv((*depthSP)["mvpMat"], 1, false, glm::value_ptr(mvp));

        for (const auto& shape : model->shapes)
        {
            glBindVertexArray(shape.vao);
            glDrawElements(GL_TRIANGLES, shape.drawCount, GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);
        }
    }
}

glm::mat4 DirectionalShadowMapper::getShadowSBPVMat()
{
    glm::mat4 scaleBiasMat =
            glm::translate(glm::mat4(1.0f), glm::vec3(0.5f, 0.5f, 0.5f)) *
            glm::scale(glm::mat4(1.0f), glm::vec3(0.5f, 0.5f, 0.5f));
    return scaleBiasMat * lightProjMat * lightViewMat;
}
