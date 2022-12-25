#pragma once


#include <assimp/material.h>
#include <glm/vec3.hpp>

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

    glm::vec3 ambient{};
    glm::vec3 diffuse{};
    glm::vec3 specular{};
    float shininess{};

    bool hasTex{false};

    Material() = default;
    
    void extractColorCoef(aiMaterial* aiMaterial);
    void bindTexture(const char* path);
};
