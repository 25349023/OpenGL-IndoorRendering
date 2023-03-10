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

    bool isEmissive = false;
    
    Model() = default;
    Model(const char* mesh_path, const char* asset_root);
    static Model quad();

    void loadMeshes(const char* path);
    void loadMaterials(const char* path);
    void setTransform(glm::vec3 t, glm::vec3 r, glm::vec3 s);
    void setEmissive(glm::vec3 em);
    void setDefaultMaterial();
    
    void render(ShaderProgram* shaderProgram, bool normalMapEnabled);

    std::pair<glm::mat4, glm::mat4> getModelMat();
};

