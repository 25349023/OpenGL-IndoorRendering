#include "DeferredRenderer.h"

#include <GLM/gtc/type_ptr.hpp>

DeferredRenderer::DeferredRenderer(glm::ivec2 ws)
{
    updateWindowSize(ws);
    glEnable(GL_DEPTH_TEST);
    recalculateCameraLocals();
}

void DeferredRenderer::setupFrameBuffer()
{
    // vao for framebuffer shader
    glGenVertexArrays(1, &frameVao);
    glBindVertexArray(frameVao);

    const float windowVertex[] =
    {
        //vec2 position vec2 texture_coord
        1.0f, 1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f,
        1.0f, -1.0f, 1.0f, 0.0f
    };

    glGenBuffers(1, &windowVbo);
    glBindBuffer(GL_ARRAY_BUFFER, windowVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(windowVertex), windowVertex, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (void*)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (void*)(sizeof(float) * 2));

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    // setup framebuffer
    glGenFramebuffers(2, fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo[0]);

    // Create fboDataTexture
    attachedTexs.clear();
    drawBuffers.clear();
    for (int i = 0; i < GBUFFER_COUNT; ++i)
    {
        attachNewFBTexture();
    }
    activeTex = RENDER_RESULT;

    enableFeature.fill(true);

    // Create Depth RBO
    glGenRenderbuffers(1, &depthRbo);
    glBindRenderbuffer(GL_RENDERBUFFER, depthRbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, winSize.x, winSize.y);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRbo);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo[1]);
    genFBTexture(secondOutputTex, 0);

    // setup fbo for volumetric light
    glGenFramebuffers(1, &volFbo);
    glBindFramebuffer(GL_FRAMEBUFFER, volFbo);
    genFBTexture(volTex, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindVertexArray(0);
}

void DeferredRenderer::teardownFrameBuffer()
{
    glDeleteVertexArrays(1, &frameVao);
    glDeleteBuffers(1, &windowVbo);
    glDeleteFramebuffers(2, fbo);
    glDeleteRenderbuffers(1, &depthRbo);
    glDeleteTextures(attachedTexs.size() - 1, attachedTexs.data() + 1);
    glDeleteTextures(1, &secondOutputTex);
}

glm::vec2 DeferredRenderer::getScreenCoord(glm::vec3 pos)
{
    glm::ivec4 viewport(0, 0, 1, 1);
    
    glm::vec4 vPrime = projMat * viewMat * glm::vec4(pos, 1.0);
    vPrime /= vPrime.w;
    
    glm::vec2 result;
    result.x = viewport.x + (viewport.z * vPrime.x + 1) / 2.0;
    result.y = viewport.y + (viewport.w * vPrime.y + 1) / 2.0;
    return result;
}

void DeferredRenderer::genFBTexture(GLuint& tex, int attachment)
{
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F,
        winSize.x, winSize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachment, GL_TEXTURE_2D, tex, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "Error binding frame buffer: " << std::hex <<
                glCheckFramebufferStatus(GL_FRAMEBUFFER) << std::endl;
    }
}

void DeferredRenderer::updateWindowSize(glm::ivec2 ws)
{
    winSize = ws;
    glViewport(0, 0, ws.x, ws.y);
    teardownFrameBuffer();
    setupFrameBuffer();

    if (gaussianBlurrer)
    {
        gaussianBlurrer->teardownFrameBuffer();
        gaussianBlurrer->setupFrameBuffer(ws);
    }
    if (sobelEdgeDetection)
    {
        sobelEdgeDetection->teardownFrameBuffer();
        sobelEdgeDetection->setupFrameBuffer(ws);
    }
    if (FXAAer) 
    {
        FXAAer->teardownFrameBuffer();
        FXAAer->setupFrameBuffer(ws);
    }
}

void DeferredRenderer::appendSceneObj(Model* model)
{
    sceneObjects.push_back(model);
}

void DeferredRenderer::attachNewFBTexture()
{
    attachedTexs.push_back(0);
    int idx = attachedTexs.size() - 1;
    if (idx == 0)
    {
        return;
    }

    // - 1 to skip the RENDER_RESULT, be careful dealing with COLOR_ATTACHMENT 
    genFBTexture(attachedTexs[idx], idx - 1);
    drawBuffers.push_back(GL_COLOR_ATTACHMENT0 + idx - 1);
}

void DeferredRenderer::rotateCamera(const float rad)
{
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


void DeferredRenderer::shadowMapStage()
{
    if (enableFeature[DIR_SHADOW_MAPPING])
    {
        dirShadowMapper->beforeRender();
        clear();
        dirShadowMapper->renderShadowMap(sceneObjects);
    }

    if (enableFeature[POINT_SHADOW_MAPPING])
    {
        pointShadowMapper->beforeRender();
        clear();
        pointShadowMapper->renderShadowMap(sceneObjects);
    }

    glViewport(0, 0, winSize.x, winSize.y);
}

void DeferredRenderer::firstStage()
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbo[0]);
    glDrawBuffers(attachedTexs.size() - 1, drawBuffers.data());
    clear();
    fbufSP->useProgram();

    glUniformMatrix4fv((*fbufSP)["projMat"], 1, false, glm::value_ptr(this->projMat));
    glUniformMatrix4fv((*fbufSP)["viewMat"], 1, false, glm::value_ptr(this->viewMat));

    glUniform3fv((*fbufSP)["directionalLight"], 1,
        glm::value_ptr(dirShadowMapper->lightEye - dirShadowMapper->lightLookAt));

    for (auto model : sceneObjects)
    {
        if (!(enableFeature[POINT_LIGHT] && enableFeature[BLOOM_EFFECT]) && model->isEmissive)
        {
            continue;
        }
        model->render(fbufSP, enableFeature[NORMAL_MAPPING]);
    }

    if (enableFeature[AREA_LIGHT] && enableFeature[BLOOM_EFFECT])
    {
        areaLight->quad.render(fbufSP, enableFeature[NORMAL_MAPPING]);
    }

    if (enableFeature[VOLUMETRIC_LIGHT])
    {
        glUniform1i((*fbufSP)["isLight"], true);
        volLight->render(fbufSP, enableFeature[NORMAL_MAPPING]);
        glUniform1i((*fbufSP)["isLight"], false);
    }

    if (enableFeature[BLOOM_EFFECT])
    {
        blurredTex = gaussianBlurrer->renderBlur(frameVao, attachedTexs[EMISSION_MAP]);
    }
}

void DeferredRenderer::secondStage()
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbo[1]);
    clear();
    screenSP->useProgram();

    glBindVertexArray(frameVao);
    glUniform1i((*screenSP)["activeTex"], activeTex);
    glUniform3fv((*screenSP)["cameraEye"], 1, glm::value_ptr(camEye));
    glUniform3fv((*screenSP)["directionalLight"], 1, glm::value_ptr(dirShadowMapper->lightEye));

    glActiveTexture(GL_TEXTURE11);
    glBindTexture(GL_TEXTURE_2D, dirShadowMapper->depthTex);
    glUniform1i((*screenSP)["shadowTex"], 11);

    glActiveTexture(GL_TEXTURE12);
    glBindTexture(GL_TEXTURE_2D, attachedTexs[EMISSION_MAP]);
    glUniform1i((*screenSP)["beforeBloomTex"], 12);

    glActiveTexture(GL_TEXTURE13);
    glBindTexture(GL_TEXTURE_CUBE_MAP, pointShadowMapper->depthTex);
    glUniform1i((*screenSP)["shadowCubeTex"], 13);
    glUniform1f((*screenSP)["far"], 10.0f);

    glUniformMatrix4fv((*screenSP)["shadowMat"], 1, false,
        glm::value_ptr(dirShadowMapper->getShadowSBPVMat()));

    glUniform3fv((*screenSP)["pointLight"], 1, glm::value_ptr(pointShadowMapper->lightPos));
    glUniform3fv((*screenSP)["pointLightAttenuation"], 1, glm::value_ptr(pointShadowMapper->lightAttenuation));
    glUniform3fv((*screenSP)["pointLightColor"], 1, glm::value_ptr(pointShadowMapper->lightColor));

    glActiveTexture(GL_TEXTURE14);
    glBindTexture(GL_TEXTURE_2D, areaLight->ltc1);
    glUniform1i((*screenSP)["LTC1"], 14);

    glActiveTexture(GL_TEXTURE15);
    glBindTexture(GL_TEXTURE_2D, areaLight->ltc2);
    glUniform1i((*screenSP)["LTC2"], 15);

    auto& quadVertex = areaLight->quad.shapes[0].vertices;
    glm::mat4 quadModelMat = areaLight->quad.getModelMat().first;
    for (int i = 0; i < 4; ++i)
    {
        std::string uniformName = std::string("areaLightPoints[") + std::to_string(i) + "]";
        glm::vec3 worldPos = glm::vec3(quadModelMat * glm::vec4(quadVertex[i].position, 1.0));
        glUniform3fv((*screenSP)[uniformName], 1, glm::value_ptr(worldPos));
    }
    glUniform3fv((*screenSP)["areaLightColor"], 1, glm::value_ptr(areaLight->quad.materials[0].emission));

    for (int i = 0; i < FEATURE_COUNT; ++i)
    {
        char uniformName[24];
        sprintf(uniformName, "enableFeature[%d]", i);
        glUniform1i((*screenSP)[uniformName], enableFeature[i]);
    }

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, attachedTexs[DIFFUSE_COLOR]);
    glUniform1i(0, 0);

    for (int i = 1; i < GBUFFER_COUNT; ++i)
    {
        glUniform1i(i, i);
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, (i == EMISSION_MAP) ? blurredTex : attachedTexs[i]);
    }
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glBindVertexArray(0);
}

void DeferredRenderer::thirdStage() {
    GLuint curTex = volTex;

    if (enableFeature[NON_PHOTOREALISTIC_RENDERING]) {
        curTex = sobelEdgeDetection->render(frameVao, curTex);
    }
    if (enableFeature[FXAA]) {
        curTex = FXAAer->render(frameVao, curTex);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    clear();
    postScreenSP->useProgram();

    glBindVertexArray(frameVao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, curTex);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glBindVertexArray(0);
}

void DeferredRenderer::volumetricLightStage()
{
    glBindFramebuffer(GL_FRAMEBUFFER, volFbo);
    clear();
    volSP->useProgram();
    glBindVertexArray(frameVao);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, secondOutputTex);
    glUniform1i((*volSP)["scene"], 1);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, attachedTexs[SCATTERING_MAP]);
    glUniform1i((*volSP)["scatterMap"], 0);

    glUniform1f((*volSP)["exposure"], 0.2f);
    glUniform1f((*volSP)["decay"], 0.96815f);
    glUniform1f((*volSP)["density"], 0.926f);
    glUniform1f((*volSP)["weight"], 0.58767f);
    glUniform1f((*volSP)["sampleWeight"], 0.3f);
    glUniform1i((*volSP)["enable"], enableFeature[VOLUMETRIC_LIGHT] && activeTex == RENDER_RESULT);

    glm::vec2 volLightScreenCoord = getScreenCoord(volLight->translation);
    glUniform2fv((*volSP)["lightPositionOnScreen"], 1, glm::value_ptr(volLightScreenCoord));
    
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glBindVertexArray(0);
}

void DeferredRenderer::clear()
{
    static const float COLOR[] = { 0.1, 0.1, 0.1, 1.0 };
    static const float NIGHT[] = { 0.19, 0.19, 0.19, 1.0 };
    static const float BLACK[] = { 0.0, 0.0, 0.0, 1.0 };
    static const float DEPTH[] = { 1.0 };

    for (int i = 1; i < attachedTexs.size(); ++i)
    {
        glClearBufferfv(GL_COLOR, i - 1, (i == EMISSION_MAP) ? BLACK : (i == SCATTERING_MAP) ? NIGHT : COLOR);
    }
    glClearBufferfv(GL_DEPTH, 0, DEPTH);
}
