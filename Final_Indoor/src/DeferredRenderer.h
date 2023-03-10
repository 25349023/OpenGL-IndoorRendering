#pragma once

#include <array>
#include <vector>
#include <glad/glad.h>

#include "AreaLight.h"
#include "DirectionalShadowMapper.h"
#include "GaussianBlurrer.h"
#include "PostShader.h"
#include "Model.h"
#include "PointShadowMapper.h"
#include "Shader.h"
#include "GLM/glm.hpp"
#include "glm/gtx/quaternion.hpp"

enum GBuffer
{
    RENDER_RESULT,
    WORLD_VERTEX,
    WORLD_NORMAL,
    AMBIENT_COLOR,
    DIFFUSE_COLOR,
    SPECULAR_COLOR,
    SHININESS,
    EMISSION_MAP,
    SCATTERING_MAP,
    GBUFFER_COUNT
};

enum Feature
{
    BLINN_PHONG_SHADING,
    DIR_SHADOW_MAPPING,
    NORMAL_MAPPING,
    POINT_LIGHT,
    BLOOM_EFFECT,
    POINT_SHADOW_MAPPING,
    AREA_LIGHT,
    NON_PHOTOREALISTIC_RENDERING,
    FXAA,
    VOLUMETRIC_LIGHT,
    FEATURE_COUNT
};

class DeferredRenderer
{
public:
    DeferredRenderer(glm::ivec2 ws);

    void updateWindowSize(glm::ivec2 ws);
    void appendSceneObj(Model* model);

    void translateCamera(glm::vec3 amount);
    void rotateCamera(const float rad);
    void recalculateCameraLocals();
    void updateViewMat();

    void shadowMapStage();
    void firstStage();
    void secondStage();
    void thirdStage();
    void volumetricLightStage();

    void clear();

    friend class MyImGuiPanel;

    glm::vec3 camEye{ 4.0, 1.0, -1.5 };
    glm::vec3 camCenter{ 3.0, 1.0, -1.5 };
    glm::vec3 camUp{ 0.0, 1.0, 0.0 };
    glm::vec3 camLocalY{ 0, 1, 0 };
    glm::vec3 camLocalZ{ 0, 0, -1 };

    // [TODO] make fbufSP, screenSP private and initialize through the ctor
    ShaderProgram* fbufSP{};
    ShaderProgram* screenSP{};
    ShaderProgram* postScreenSP{};
    ShaderProgram* volSP{};

    glm::mat4 projMat{ 1.0 };
    glm::mat4 viewMat{ 1.0 };

    DirectionalShadowMapper* dirShadowMapper{};
    PointShadowMapper* pointShadowMapper{};
    GaussianBlurrer* gaussianBlurrer{};
    AreaLight* areaLight{};
    PostShader* sobelEdgeDetection{};
    PostShader* FXAAer{};

    Model* volLight{};

private:
    GLuint frameVao{};
    GLuint windowVbo{};
    GLuint fbo[2]{};
    GLuint volFbo{};
    GLuint depthRbo{};

    std::vector<Model*> sceneObjects;

    std::vector<GLuint> attachedTexs{};
    std::vector<GLenum> drawBuffers{};
    GLuint activeTex{};
    GLuint blurredTex{};
    GLuint secondOutputTex{};
    GLuint volTex{};

    std::array<bool, FEATURE_COUNT> enableFeature{};

    glm::ivec2 winSize{};

    void attachNewFBTexture();
    void setupFrameBuffer();
    void teardownFrameBuffer();
    glm::vec2 getScreenCoord(glm::vec3 pos);
    void genFBTexture(GLuint& tex, int attachment);
};
