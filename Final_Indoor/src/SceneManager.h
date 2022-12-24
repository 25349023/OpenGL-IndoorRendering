#pragma once

#include <glm\mat4x4.hpp>
#include <glm\gtx\transform.hpp>
#include <glm\gtc\type_ptr.hpp>


#include <glad\glad.h>

// Singleton

class SceneManager
{
private:
    SceneManager() {}


public:
    virtual ~SceneManager() {}

    static SceneManager* Instance()
    {
        static SceneManager* m_instance = nullptr;
        if (m_instance == nullptr)
        {
            m_instance = new SceneManager();
        }
        return m_instance;
    }

    GLuint m_vertexHandle;
    GLuint m_normalHandle;
    GLuint m_uvHandle;
    GLuint m_offsetHandel;

    GLuint m_projMatHandle;
    GLuint m_viewMatHandle;
    GLuint m_modelMatHandle;
    GLuint m_instancedDrawHandle;

    GLuint m_materialHandle;

    GLuint m_fs_pixelProcessIdHandle;
    GLuint m_fs_albedoTexHandle;

    GLuint m_fs_kaHandle;
    GLuint m_fs_kdHandle;
    GLuint m_fs_ksHandle;

    GLenum m_albedoTexUnit;
    GLenum m_normalTexUnit;

    int m_fs_commonProcess;
    int m_fs_textureMapping;
    int m_fs_simpleShading;
};
