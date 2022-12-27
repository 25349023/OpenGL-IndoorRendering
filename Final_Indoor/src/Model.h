#pragma once
#include <vector>
#include <assimp/scene.h>

#include "Shape.h"
#include "Material.h"
#include "Shader.h"

struct Model
{
    const aiScene* model{};
    std::vector<Shape> shapes{};
    std::vector<Material> materials{};

    std::vector<unsigned int> drawCounts;
    std::vector<int> baseVertices;

    glm::vec3 translation{};
    glm::vec3 rotation{};
    glm::vec3 scaling{};

    Model() = default;
    Model(const char* mesh_path, const char* asset_root);

    void loadMeshes(const char* path);
    void loadMaterials(const char* path);
    void setTransform(glm::vec3 t, glm::vec3 r, glm::vec3 s);
    void render(ShaderProgram* shaderProgram, bool normalMapEnabled);

    std::pair<glm::mat4, glm::mat4> getModelMat();

    static Model merge(std::vector<Model>& models);
};

