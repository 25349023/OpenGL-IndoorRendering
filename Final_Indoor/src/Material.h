#pragma once


#include <assimp/material.h>
#include <GLM/glm.hpp>

#include "glad/glad.h"

typedef struct _texture_data
{
    _texture_data() : width(0), height(0), data(0) {}
    int width;
    int height;
    unsigned char* data;
} texture_data;

texture_data loadImg(const char* path);

struct Material
{
    texture_data texture;
    GLuint diffuseTex{};
    GLuint normalTex{};

    glm::vec3 ambient{};
    glm::vec3 diffuse{};
    glm::vec3 specular{};
    float shininess{};
    glm::vec3 emission{};

    bool hasTex{false};
    bool hasNorm{false};
    bool isEmissive{false};

    Material() = default;
    
    void extractColorCoef(aiMaterial* aiMaterial);
    void bindTexture(const char* path, bool isNormalMap = false);

    void setEmissive(glm::vec3 em);
};
