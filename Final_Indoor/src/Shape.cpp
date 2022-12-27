#include "Shape.h"

#include <iostream>

#include "SceneManager.h"

void Shape::extractMeshData(const aiMesh* mesh)
{
    for (unsigned int v = 0; v < mesh->mNumVertices; ++v)
    {
        Vertex vertex;
        vertex.position = glm::vec3(mesh->mVertices[v].x, mesh->mVertices[v].y, mesh->mVertices[v].z);
        vertex.normal = glm::vec3(mesh->mNormals[v].x, mesh->mNormals[v].y, mesh->mNormals[v].z);
        if (mesh->HasTextureCoords(0))
        {
            vertex.texCoords = glm::vec3(mesh->mTextureCoords[0][v].x, mesh->mTextureCoords[0][v].y, 0.0);
        }
        if (mesh->HasTangentsAndBitangents())
        {
            vertex.tangent = glm::vec3(mesh->mTangents[v].x, mesh->mTangents[v].y, mesh->mTangents[v].z);
            vertex.bitangent = glm::vec3(mesh->mBitangents[v].x, mesh->mBitangents[v].y, mesh->mBitangents[v].z);
        }
        vertices.push_back(vertex);
    }
}

void Shape::extractMeshIndices(const aiMesh* mesh)
{
    for (unsigned int f = 0; f < mesh->mNumFaces; ++f)
    {
        if (mesh->mFaces[f].mNumIndices < 3)
        {
            continue;
        }

        unsigned int* idx = mesh->mFaces[f].mIndices;
        indices.insert(indices.end(), idx, idx + 3);
    }
}

void Shape::bindBuffers()
{
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // vertex buffer
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

    SceneManager* sm = SceneManager::Instance();
    glVertexAttribPointer(sm->m_vertexHandle, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glVertexAttribPointer(sm->m_normalHandle, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glVertexAttribPointer(sm->m_uvHandle, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));
    glVertexAttribPointer(sm->m_tangentHandle, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tangent));
    glVertexAttribPointer(sm->m_bitangentHandle, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, bitangent));

    glEnableVertexAttribArray(sm->m_vertexHandle);
    glEnableVertexAttribArray(sm->m_normalHandle);
    glEnableVertexAttribArray(sm->m_uvHandle);
    glEnableVertexAttribArray(sm->m_tangentHandle);
    glEnableVertexAttribArray(sm->m_bitangentHandle);

    // index buffer
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(), indices.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);
}
