#pragma once

#include <glad\glad.h>

// Singleton

class SceneManager
{
private:
    SceneManager(): m_vertexHandle(0), m_normalHandle(1), m_uvHandle(2),
    m_fs_commonProcess(0), m_fs_textureMapping(1), m_fs_simpleShading(2) {}


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

    int m_fs_commonProcess;
    int m_fs_textureMapping;
    int m_fs_simpleShading;
};
