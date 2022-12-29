#include "PointShadowMapper.h"

PointShadowMapper::PointShadowMapper(ShaderProgram* sp)
{
    depthSP = sp;
    setupDepthFrameBuffer();
    updateLightVPMat();
}

void PointShadowMapper::setupDepthFrameBuffer()
{
    glGenFramebuffers(1, &depthFbo);
    glBindFramebuffer(GL_FRAMEBUFFER, depthFbo);

    glGenTextures(1, &depthTex);
    glBindTexture(GL_TEXTURE_CUBE_MAP, depthTex);

    for (int i = 0; i < 6; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT,
            shadowMapRes, shadowMapRes, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTex, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "Error binding depth frame buffer: " << std::hex <<
                glCheckFramebufferStatus(GL_FRAMEBUFFER) << std::endl;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PointShadowMapper::updateLightVPMat()
{
    float aspect = (float)shadowMapRes / (float)shadowMapRes;
    float near = 1.0f;
    float far = 10.0f;
    glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), aspect, near, far);

    lightPVMats.clear();
    lightPVMats.push_back(
        shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
    lightPVMats.push_back(
        shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
    lightPVMats.push_back(
        shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)));
    lightPVMats.push_back(
        shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0)));
    lightPVMats.push_back(
        shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0)));
    lightPVMats.push_back(
        shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0)));
}

void PointShadowMapper::beforeRender()
{
    glBindFramebuffer(GL_FRAMEBUFFER, depthFbo);
    glDrawBuffer(GL_NONE);
    glViewport(0, 0, shadowMapRes, shadowMapRes);
    depthSP->useProgram();

    updateLightVPMat();
}

void PointShadowMapper::renderShadowMap(const std::vector<Model*>& sceneObjs)
{
    for (int i = 0; i < lightPVMats.size(); ++i)
    {
        std::string uniformName = std::string("pvMat[") + std::to_string(i) + "]";
        glUniformMatrix4fv((*depthSP)[uniformName], 1, false,
            glm::value_ptr(lightPVMats[i]));
    }

    glUniform1f((*depthSP)["far"], 10.0f);
    glUniform3fv((*depthSP)["lightPos"], 1, glm::value_ptr(lightPos));

    for (auto model : sceneObjs)
    {
        glm::mat4 modelMat = model->getModelMat().first;

        glUniformMatrix4fv((*depthSP)["modelMat"], 1, false, glm::value_ptr(modelMat));

        for (const auto& shape : model->shapes)
        {
            glBindVertexArray(shape.vao);
            glDrawElements(GL_TRIANGLES, shape.drawCount, GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);
        }
    }
}
