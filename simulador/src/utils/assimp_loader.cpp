// NEW FILE: src/utils/assimp_loader.cpp
#include "assimp_loader.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>

namespace Utils
{

  std::unique_ptr<Scene::Model> AssimpLoader::loadModel(
      const std::string &filepath,
      const glm::vec3 &uniformColor)
  {
    // Crear importador de Assimp
    Assimp::Importer importer;

    // Leer el archivo con opciones de post-procesamiento
    const aiScene *scene = importer.ReadFile(filepath,
                                              aiProcess_Triangulate |           // Convertir a triángulos
                                              aiProcess_GenNormals |            // Generar normales si no existen
                                              aiProcess_FlipUVs |               // Voltear coordenadas UV
                                              aiProcess_CalcTangentSpace |      // Calcular tangentes
                                              aiProcess_JoinIdenticalVertices); // Optimizar vértices

    // Verificar errores
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
      std::cerr << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
      return nullptr;
    }

    // Crear modelo
    auto model = std::make_unique<Scene::Model>(filepath);

    // Procesar el nodo raíz recursivamente
    processNode(scene->mRootNode, scene, model.get(), uniformColor);

    std::cout << "Model loaded with Assimp: " << filepath << std::endl;
    std::cout << "  Meshes: " << model->getMeshCount() << std::endl;
    std::cout << "  Uniform color: RGB(" << uniformColor.r << ", "
              << uniformColor.g << ", " << uniformColor.b << ")" << std::endl;

    return model;
  }

  void AssimpLoader::processNode(
      aiNode *node,
      const aiScene *scene,
      Scene::Model *model,
      const glm::vec3 &uniformColor)
  {
    // Procesar todas las mallas del nodo
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
      aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
      auto processed_mesh = processMesh(mesh, scene, uniformColor);
      if (processed_mesh)
      {
        model->addMesh(std::move(processed_mesh));
      }
    }

    // Procesar todos los nodos hijos recursivamente
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
      processNode(node->mChildren[i], scene, model, uniformColor);
    }
  }

  std::unique_ptr<Graphics::Rendering::Mesh> AssimpLoader::processMesh(
      aiMesh *mesh,
      const aiScene *scene,
      const glm::vec3 &uniformColor)
  {
    std::vector<Graphics::Rendering::Vertex> vertices;
    std::vector<unsigned int> indices;

    // Procesar vértices
    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
      Graphics::Rendering::Vertex vertex;

      // Posición
      vertex.position = glm::vec3(
          mesh->mVertices[i].x,
          mesh->mVertices[i].y,
          mesh->mVertices[i].z);

      // Normales
      if (mesh->HasNormals())
      {
        vertex.normal = glm::vec3(
            mesh->mNormals[i].x,
            mesh->mNormals[i].y,
            mesh->mNormals[i].z);
      }
      else
      {
        vertex.normal = glm::vec3(0.0f, 1.0f, 0.0f); // Normal por defecto
      }

      // Coordenadas de textura (primera capa si existe)
      if (mesh->mTextureCoords[0])
      {
        vertex.texture_coords = glm::vec2(
            mesh->mTextureCoords[0][i].x,
            mesh->mTextureCoords[0][i].y);
      }
      else
      {
        vertex.texture_coords = glm::vec2(0.0f, 0.0f);
      }

      // Tangentes
      if (mesh->HasTangentsAndBitangents())
      {
        vertex.tangent = glm::vec3(
            mesh->mTangents[i].x,
            mesh->mTangents[i].y,
            mesh->mTangents[i].z);

        vertex.bitangent = glm::vec3(
            mesh->mBitangents[i].x,
            mesh->mBitangents[i].y,
            mesh->mBitangents[i].z);
      }
      else
      {
        vertex.tangent = glm::vec3(1.0f, 0.0f, 0.0f);
        vertex.bitangent = glm::vec3(0.0f, 0.0f, 1.0f);
      }

      vertices.push_back(vertex);
    }

    // Procesar índices
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
      aiFace face = mesh->mFaces[i];
      for (unsigned int j = 0; j < face.mNumIndices; j++)
      {
        indices.push_back(face.mIndices[j]);
      }
    }

    // Crear y retornar la malla
    return std::make_unique<Graphics::Rendering::Mesh>(
        vertices, indices, mesh->mName.C_Str());
  }

} // namespace Utils
