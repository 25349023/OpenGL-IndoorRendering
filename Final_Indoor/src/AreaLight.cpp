#include "AreaLight.h"
#include "ltc_matrix.hpp" // contains float arrays LTC1 and LTC2

AreaLight::AreaLight(glm::vec3 position, glm::vec3 rotation, glm::vec2 scaling, glm::vec3 color)
    : lightPos(position), lightRot(rotation), lightScale(scaling), lightColor(color)
{
    quad = Model::quad();
    quad.setDefaultMaterial();
    updateParameters();

    setupLtcTexture();
}

GLuint loadLtcTexture(const float* matrixTable)
{
    GLuint texture = 0;

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 64, 64, 0, GL_RGBA, GL_FLOAT, matrixTable);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);
    return texture;
}

void AreaLight::setupLtcTexture()
{
    ltc1 = loadLtcTexture(LTC1);
    ltc2 = loadLtcTexture(LTC2);
}

void AreaLight::updateParameters()
{
    quad.setTransform(lightPos, lightRot, glm::vec3(lightScale.x, 1.0, lightScale.y));
    quad.setEmissive(lightColor);
}
