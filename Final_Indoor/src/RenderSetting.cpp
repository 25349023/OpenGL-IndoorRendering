#include "RenderSetting.h"


RenderSetting::RenderSetting() {}

RenderSetting::~RenderSetting() {}

void RenderSetting::prepareUniform()
{
    glUniformMatrix4fv((*m_shaderProgram)["projMat"], 1, false, glm::value_ptr(this->m_projMat));
    glUniformMatrix4fv((*m_shaderProgram)["viewMat"], 1, false, glm::value_ptr(this->m_viewMat));
}

void RenderSetting::useProgram()
{
    this->m_shaderProgram->useProgram();
}

// =======================================
void RenderSetting::resize(const int w, const int h)
{
    this->m_frameWidth = w;
    this->m_frameHeight = h;
    setViewport(0, 0, this->m_frameWidth, this->m_frameHeight);
}

bool RenderSetting::initialize(const int w, const int h, ShaderProgram* shaderProgram)
{
    this->m_shaderProgram = shaderProgram;

    this->resize(w, h);
    const bool flag = this->setUpShader();

    if (!flag)
    {
        return false;
    }

    glEnable(GL_DEPTH_TEST);

    return true;
}

void RenderSetting::setProjection(const glm::mat4& proj)
{
    this->m_projMat = proj;
}

void RenderSetting::setView(const glm::mat4& view)
{
    this->m_viewMat = view;
}

void RenderSetting::setViewport(const int x, const int y, const int w, const int h)
{
    glViewport(x, y, w, h);
}

bool RenderSetting::setUpShader()
{
    if (this->m_shaderProgram == nullptr)
    {
        return false;
    }

    this->m_shaderProgram->useProgram();

    SceneManager* manager = SceneManager::Instance();
    manager->m_vertexHandle = 0;
    manager->m_normalHandle = 1;
    manager->m_uvHandle = 2;

    glUniform1i((*m_shaderProgram)["albedoTexture"], 0);

    manager->m_albedoTexUnit = GL_TEXTURE0;

    manager->m_fs_commonProcess = 0;
    manager->m_fs_textureMapping = 1;
    manager->m_fs_simpleShading = 2;

    return true;
}
