#pragma once

#include "Shader.h"
#include "SceneManager.h"


class RenderSetting
{
public:
    RenderSetting();
    virtual ~RenderSetting();
    
    friend class MyImGuiPanel;
    friend class DeferredRenderer;

private:
    ShaderProgram* m_shaderProgram = nullptr;
    glm::mat4 m_projMat;
    glm::mat4 m_viewMat;
    
    int m_frameWidth;
    int m_frameHeight;

public:
    void resize(const int w, const int h);
    bool initialize(const int w, const int h, ShaderProgram* shaderProgram);

    void setProjection(const glm::mat4& proj);
    void setView(const glm::mat4& view);
    void setViewport(const int x, const int y, const int w, const int h);

public:
    void prepareUniform();
    void useProgram();

private:
    bool setUpShader();
};
