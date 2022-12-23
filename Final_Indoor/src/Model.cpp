#include "Model.h"

#include <iostream>
#include <assimp/cimport.h>
#include <assimp/postprocess.h>


Model::Model(const char* mesh_path, const char* asset_root)
{
    loadMeshes(mesh_path);
    loadMaterials(asset_root);
    
    aiReleaseImport(model);
    model = nullptr;
}

void Model::loadMeshes(const char* path)
{
    model = aiImportFile(path,
        aiProcess_CalcTangentSpace |
        aiProcess_Triangulate |
        aiProcess_GenNormals |
        aiProcess_JoinIdenticalVertices
    );

    std::cout << "There are " << model->mNumMeshes << " meshes." << std::endl;
    std::cout << "There are " << model->mNumMaterials << " materials." << std::endl;

    // [TODO] extract from multiple meshes
    for (unsigned int i = 0; i < model->mNumMeshes; ++i)
    {
        const aiMesh* mesh = model->mMeshes[i];
        Shape shape;

        shape.extractMeshData(mesh);
        shape.extractMeshIndices(mesh);

        shape.bindBuffers();

        shape.materialId = mesh->mMaterialIndex;
        shape.drawCount = mesh->mNumFaces * 3;

        shapes.push_back(shape);
    }
}

void Model::loadMaterials(const char* asset_root)
{
    for (unsigned int i = 0; i < model->mNumMaterials; ++i)
    {
        aiMaterial* aiMaterial = model->mMaterials[i];
        Material material;

        material.extractColorCoef(aiMaterial);

        aiString texturePath;
        if (aiMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) == aiReturn_SUCCESS)
        {
            char full_path[96] = "";
            strncpy(full_path, asset_root, 45);
            strncat(full_path, texturePath.C_Str(), 45);

            material.bindTexture(full_path);
        }

        materials.push_back(material);
    }
}

Model Model::merge(std::vector<Model>& models)
{
    Model merged;
    for (auto& model : models)
    {
        merged.shapes.insert(merged.shapes.end(), model.shapes.begin(), model.shapes.end());
        merged.materials.insert(merged.materials.end(), model.materials.begin(), model.materials.end());
    }
    
    return merged;

    // merge shapes
    // auto& shape = merged.shape;
    // for (int i = 0; i < models.size(); ++i)
    // {
    //     merged.baseVertices.push_back(shape.vertices.size());
    //
    //     for (Vertex& v : models[i].shape.vertices)
    //     {
    //         v.tex_coords.z = (float)i;
    //     }
    //
    //     shape.vertices.insert(shape.vertices.end(),
    //         models[i].shape.vertices.begin(), models[i].shape.vertices.end());
    //     shape.indices.insert(shape.indices.end(),
    //         models[i].shape.indices.begin(), models[i].shape.indices.end());
    //
    //     merged.drawCounts.push_back(models[i].shape.drawCount);
    //     shape.drawCount += models[i].shape.drawCount;
    // }
    //
    // shape.bindBuffers();
    //
    // // merge textures
    // auto& texture = merged.material.texture;
    // texture.height = models[0].material.texture.height;
    // texture.width = models[0].material.texture.width;
    //
    // size_t texture_size = texture.width * texture.height * 4;
    // texture.data = new unsigned char[texture_size * models.size()];
    //
    // for (int i = 0; i < models.size(); ++i)
    // {
    //     memcpy(texture.data + texture_size * i, models[i].material.texture.data, texture_size);
    // }
    //
    // merged.material.bindTexture2DArray(models.size());

    // return merged;
}
