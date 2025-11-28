// NEW FILE: src/utils/model_loader.cpp
#include "model_loader.h"
#include "../scene/mesh.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>
#include <limits>

namespace Utils
{

  std::unique_ptr<Scene::Model> ModelLoader::loadModel(const std::string &filepath,
                                                       const std::string &model_name)
  {
    // Crear importador de Assimp
    Assimp::Importer importer;

    // Leer el archivo con opciones de post-procesado
    const aiScene *scene = importer.ReadFile(filepath,
                                             aiProcess_Triangulate |               // Convertir a triángulos
                                                 aiProcess_FlipUVs |               // Voltear coordenadas UV
                                                 aiProcess_GenNormals |            // Generar normales si no existen
                                                 aiProcess_CalcTangentSpace |      // Calcular tangentes y bitangentes
                                                 aiProcess_JoinIdenticalVertices); // Optimizar vértices

    // Verificar errores
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
      std::cerr << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
      return nullptr;
    }

    std::string directory = getDirectory(filepath);
    std::string name = model_name.empty() ? filepath : model_name;

    auto model = std::make_unique<Scene::Model>(name);
    std::vector<std::unique_ptr<Graphics::Rendering::Mesh>> meshes;
    glm::vec3 model_center = calculateModelCenter(scene);
    processNode(scene->mRootNode, scene, meshes, directory, model_center);

    for (auto &mesh : meshes)
    {
      model->addMesh(std::move(mesh));
    }

    std::cout << "Modelo cargado exitosamente: " << filepath << std::endl;
    std::cout << "  Meshes: " << model->getMeshCount() << std::endl;
    std::cout << "  Centro del modelo: (" << model_center.x << ", "
              << model_center.y << ", " << model_center.z << ")" << std::endl;

    return model;
  }

  void ModelLoader::processNode(::aiNode *node,
                                const ::aiScene *scene,
                                std::vector<std::unique_ptr<Graphics::Rendering::Mesh>> &meshes,
                                const std::string &directory,
                                const glm::vec3 &center_offset)
  {
    // Procesar todos los meshes del nodo actual
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
      ::aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
      auto processed_mesh = processMesh(mesh, scene, directory, center_offset);
      if (processed_mesh)
      {
        meshes.push_back(std::move(processed_mesh));
      }
    }

    // Procesar recursivamente todos los nodos hijos
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
      processNode(node->mChildren[i], scene, meshes, directory, center_offset);
    }
  }

  std::unique_ptr<Graphics::Rendering::Mesh> ModelLoader::processMesh(::aiMesh *mesh,
                                                                      const ::aiScene * /*scene*/,
                                                                      const std::string & /*directory*/,
                                                                      const glm::vec3 &center_offset)
  {
    std::vector<Graphics::Rendering::Vertex> vertices;
    std::vector<unsigned int> indices;

    // Procesar vértices
    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
      Graphics::Rendering::Vertex vertex;

      // Posición (centrada restando el offset) - Intercambiando X y Z
      vertex.position = glm::vec3(
          mesh->mVertices[i].z - center_offset.z,  // X <- Z
          mesh->mVertices[i].y - center_offset.y,  // Y <- Y
          mesh->mVertices[i].x - center_offset.x); // Z <- X

      // Normales - Intercambiando X y Z
      if (mesh->HasNormals())
      {
        vertex.normal = glm::vec3(
            mesh->mNormals[i].z,  // X <- Z
            mesh->mNormals[i].y,  // Y <- Y
            mesh->mNormals[i].x); // Z <- X
      }
      else
      {
        vertex.normal = glm::vec3(0.0f, 1.0f, 0.0f);
      }

      // Coordenadas de textura (solo el primer set)
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

      // Tangentes - Intercambiando X y Z
      if (mesh->HasTangentsAndBitangents())
      {
        vertex.tangent = glm::vec3(
            mesh->mTangents[i].z,  // X <- Z
            mesh->mTangents[i].y,  // Y <- Y
            mesh->mTangents[i].x); // Z <- X

        vertex.bitangent = glm::vec3(
            mesh->mBitangents[i].z,  // X <- Z
            mesh->mBitangents[i].y,  // Y <- Y
            mesh->mBitangents[i].x); // Z <- X
      }
      else
      {
        vertex.tangent = glm::vec3(0.0f, 0.0f, 1.0f);   // Ajustado para nuevo sistema
        vertex.bitangent = glm::vec3(1.0f, 0.0f, 0.0f); // Ajustado para nuevo sistema
      }

      // Color del vértice (blanco por defecto)
      vertex.color = glm::vec3(1.0f, 1.0f, 1.0f);

      vertices.push_back(vertex);
    }

    // Procesar índices
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
      ::aiFace face = mesh->mFaces[i];
      for (unsigned int j = 0; j < face.mNumIndices; j++)
      {
        indices.push_back(face.mIndices[j]);
      }
    }

    // Crear el mesh
    std::string mesh_name = mesh->mName.C_Str();
    if (mesh_name.empty())
    {
      mesh_name = "mesh";
    }

    auto result_mesh = std::make_unique<Graphics::Rendering::Mesh>(vertices, indices, mesh_name);
    return result_mesh;
  }

  std::string ModelLoader::getDirectory(const std::string &filepath)
  {
    size_t last_slash = filepath.find_last_of("/\\");
    if (last_slash != std::string::npos)
    {
      return filepath.substr(0, last_slash);
    }
    return ".";
  }

  glm::vec3 ModelLoader::calculateModelCenter(const ::aiScene *scene)
  {
    glm::vec3 min_bounds(std::numeric_limits<float>::max());
    glm::vec3 max_bounds(std::numeric_limits<float>::lowest());

    for (unsigned int m = 0; m < scene->mNumMeshes; m++)
    {
      ::aiMesh *mesh = scene->mMeshes[m];

      for (unsigned int i = 0; i < mesh->mNumVertices; i++)
      {
        // Intercambiar X y Z al calcular el centro
        glm::vec3 pos(mesh->mVertices[i].z, mesh->mVertices[i].y, mesh->mVertices[i].x);

        min_bounds.x = std::min(min_bounds.x, pos.x);
        min_bounds.y = std::min(min_bounds.y, pos.y);
        min_bounds.z = std::min(min_bounds.z, pos.z);

        max_bounds.x = std::max(max_bounds.x, pos.x);
        max_bounds.y = std::max(max_bounds.y, pos.y);
        max_bounds.z = std::max(max_bounds.z, pos.z);
      }
    }

    // Retornar el centro geométrico
    return (min_bounds + max_bounds) * 0.5f;
  }

} // namespace Utils
