#pragma once

#include "Model.h"
#include "Shape.h"
#include "glad/glad.h"

class AreaLight
{
public:
    AreaLight(glm::vec3 position, glm::vec3 rotation, glm::vec2 scaling, glm::vec3 color);

    Model quad;
    GLuint ltc1{};
    GLuint ltc2{};

    glm::vec3 lightPos;
    glm::vec3 lightRot;
    glm::vec2 lightScale;
    glm::vec3 lightColor;

    void setupLtcTexture();
    void updateParameters();
};
