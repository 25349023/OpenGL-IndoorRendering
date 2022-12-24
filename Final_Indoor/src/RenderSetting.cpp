#include "RenderSetting.h"


RenderSetting::RenderSetting() {}


RenderSetting::~RenderSetting() {}

void RenderSetting::startNewFrame()
{
    this->m_shaderProgram->useProgram();
    this->clear();
}

void RenderSetting::beforeRender()
{
    SceneManager* manager = SceneManager::Instance();

    glUniformMatrix4fv(manager->m_projMatHandle, 1, false, glm::value_ptr(this->m_projMat));
    glUniformMatrix4fv(manager->m_viewMatHandle, 1, false, glm::value_ptr(this->m_viewMat));
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

void RenderSetting::clear(const glm::vec4& clearColor, const float depth)
{
    static const float COLOR[] = {0.1, 0.1, 0.1, 1.0};
    static const float DEPTH[] = {1.0};

    glClearBufferfv(GL_COLOR, 0, COLOR);
    glClearBufferfv(GL_DEPTH, 0, DEPTH);
}

bool RenderSetting::setUpShader()
{
    if (this->m_shaderProgram == nullptr)
    {
        return false;
    }

    this->m_shaderProgram->useProgram();

    // shader attributes binding
    const GLuint programId = this->m_shaderProgram->programId();

    SceneManager* manager = SceneManager::Instance();
    manager->m_vertexHandle = 0;
    manager->m_normalHandle = 1;
    manager->m_uvHandle = 2;
    manager->m_offsetHandel = 3;

    manager->m_modelMatHandle = glGetUniformLocation(programId, "modelMat");
    manager->m_viewMatHandle = glGetUniformLocation(programId, "viewMat");
    manager->m_projMatHandle = glGetUniformLocation(programId, "projMat");

    manager->m_materialHandle = glGetUniformLocation(programId, "matParams");
    GLuint albedoTexHandle = glGetUniformLocation(programId, "albedoTexture");

    glUniform1i(albedoTexHandle, 0);

    manager->m_albedoTexUnit = GL_TEXTURE0;

    manager->m_fs_pixelProcessIdHandle = glGetUniformLocation(programId, "pixelProcessId");
    manager->m_fs_commonProcess = 0;
    manager->m_fs_textureMapping = 1;
    manager->m_fs_simpleShading = 2;

    manager->m_fs_albedoTexHandle = glGetUniformLocation(programId, "albedoTex");
    manager->m_fs_kaHandle = glGetUniformLocation(programId, "ka");
    manager->m_fs_kdHandle = glGetUniformLocation(programId, "kd");
    manager->m_fs_ksHandle = glGetUniformLocation(programId, "ks");

    return true;
}
