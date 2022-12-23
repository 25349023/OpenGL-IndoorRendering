#pragma once
#include <vector>
#include <assimp/scene.h>

#include "Shape.h"
#include "Material.h"

struct Model
{
    const aiScene* model{};
    std::vector<Shape> shapes{};
    std::vector<Material> materials{};

    std::vector<unsigned int> drawCounts;
    std::vector<int> baseVertices;

    Model() = default;
    Model(const char* mesh_path, const char* asset_root);

    void loadMeshes(const char* path);
    void loadMaterials(const char* path);

    static Model merge(std::vector<Model>& models);
};

