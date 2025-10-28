// NEW FILE: src/utils/assimp_loader.cpp
#include "assimp_loader.h"
#include "../graphics/textures/texture_manager.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>
#include <array>
#include <cstdlib>
#include <glad/glad.h>

// Nota: STB_IMAGE_IMPLEMENTATION ya está definido en texture_manager.cpp
// Solo incluimos el header para usar las funciones
#include "../../include/stb_image.h"

namespace Utils
{

  // === DEBUG: Axis remap configuration ===
  // Permite reordenar e invertir ejes al importar el modelo para probar orientaciones.
  // Definición: pos_dest = SIGN[i] * src[MAP_POS[i]]
  // Preset actual: mantener X, usar Z como Y, e invertir Y->Z (common fix para GLTF Y-up vs Z-up)
  namespace {
    // Índices de origen: 0->x, 1->y, 2->z
  // Ajuste: rotación de +90° alrededor de X para que el eje +Z original pase a -Y (ruedas hacia abajo)
  // dest = (x, -z, y)
  static const int MAP_POS[3] = {0, 2, 1};     // dest.x <- src.x, dest.y <- src.z, dest.z <- src.y
  // Para corregir que el avión mire hacia atrás, aplicamos adicionalmente una rotación de 180° alrededor de Y (dest):
  // esto equivale a invertir X y Z en el espacio destino. Mantenemos Y invertida para ruedas hacia abajo.
  // Resultado final: dest = (-x, -z, -y)
  static const float MAP_SIGN[3] = {-1.0f, -1.0f, -1.0f};

    inline glm::vec3 mapVec3(const aiVector3D &v)
    {
      float s[3] = {v.x, v.y, v.z};
      return glm::vec3(
          s[MAP_POS[0]] * MAP_SIGN[0],
          s[MAP_POS[1]] * MAP_SIGN[1],
          s[MAP_POS[2]] * MAP_SIGN[2]);
    }
  }

  std::unique_ptr<Scene::Model> AssimpLoader::loadModel(
      const std::string &filepath,
      const glm::vec3 &uniformColor)
  {
    // Crear importador de Assimp
    Assimp::Importer importer;

    // Leer el archivo con opciones de post-procesamiento
    // Nota: No usar aiProcess_FlipUVs para GLB/GLTF ya que tienen su propio sistema de coordenadas
    const aiScene *scene = importer.ReadFile(filepath,
                                              aiProcess_Triangulate |           // Convertir a triángulos
                                              aiProcess_GenNormals |            // Generar normales si no existen
                                              aiProcess_CalcTangentSpace |      // Calcular tangentes
                                              aiProcess_JoinIdenticalVertices); // Optimizar vértices

    // Verificar errores
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
      std::cerr << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
      return nullptr;
    }

    // Extraer directorio del archivo
    std::string directory = filepath.substr(0, filepath.find_last_of('/'));

    // Crear modelo
    auto model = std::make_unique<Scene::Model>(filepath);

  // Log de mapeo de ejes (una vez por modelo cargado)
  std::cout << "[AssimpLoader] Axis remap in effect:" << std::endl;
  std::cout << "  dest.x = sign(" << MAP_SIGN[0] << ") * src[" << MAP_POS[0] << "]" << std::endl;
  std::cout << "  dest.y = sign(" << MAP_SIGN[1] << ") * src[" << MAP_POS[1] << "]" << std::endl;
  std::cout << "  dest.z = sign(" << MAP_SIGN[2] << ") * src[" << MAP_POS[2] << "]" << std::endl;

    // Procesar el nodo raíz recursivamente
    processNode(scene->mRootNode, scene, model.get(), uniformColor, directory);

    std::cout << "Model loaded with Assimp: " << filepath << std::endl;
    std::cout << "  Meshes: " << model->getMeshCount() << std::endl;
    std::cout << "  Materials: " << scene->mNumMaterials << std::endl;
    std::cout << "  Embedded textures: " << scene->mNumTextures << std::endl;

    return model;
  }

  void AssimpLoader::processNode(
      aiNode *node,
      const aiScene *scene,
      Scene::Model *model,
      const glm::vec3 &uniformColor,
      const std::string &directory)
  {
    // Procesar todas las mallas del nodo
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
      aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
      auto processed_mesh = processMesh(mesh, scene, uniformColor, directory);
      if (processed_mesh)
      {
        model->addMesh(std::move(processed_mesh));
      }
    }

    // Procesar todos los nodos hijos recursivamente
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
      processNode(node->mChildren[i], scene, model, uniformColor, directory);
    }
  }

  std::unique_ptr<Graphics::Rendering::Mesh> AssimpLoader::processMesh(
      aiMesh *mesh,
      const aiScene *scene,
      const glm::vec3 &uniformColor,
      const std::string &directory)
  {
    std::vector<Graphics::Rendering::Vertex> vertices;
    std::vector<unsigned int> indices;

    // Extraer color del material si está disponible
    glm::vec3 material_color = uniformColor;
    if (scene->mMaterials && mesh->mMaterialIndex >= 0)
    {
      aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
      aiColor3D diffuse_color(1.0f, 1.0f, 1.0f);
      
      if (material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse_color) == AI_SUCCESS)
      {
        material_color = glm::vec3(diffuse_color.r, diffuse_color.g, diffuse_color.b);
        std::cout << "  Material color found: RGB(" << material_color.r << ", "
                  << material_color.g << ", " << material_color.b << ")" << std::endl;
      }
    }

    // Procesar vértices
    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
      Graphics::Rendering::Vertex vertex;

      // Posición con mapeo de ejes
      vertex.position = mapVec3(mesh->mVertices[i]);

      // Normales (mismo mapeo + normalización)
      if (mesh->HasNormals())
      {
        vertex.normal = glm::normalize(mapVec3(mesh->mNormals[i]));
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
        vertex.tangent = glm::normalize(mapVec3(mesh->mTangents[i]));
        vertex.bitangent = glm::normalize(mapVec3(mesh->mBitangents[i]));
      }
      else
      {
        vertex.tangent = glm::vec3(1.0f, 0.0f, 0.0f);
        vertex.bitangent = glm::vec3(0.0f, 0.0f, 1.0f);
      }

      // Asignar color del material como color del vértice
      vertex.color = material_color;

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

    // Crear la malla
    auto processed_mesh = std::make_unique<Graphics::Rendering::Mesh>(
        vertices, indices, mesh->mName.C_Str());

    // Cargar textura del material si existe
    if (scene->mMaterials && mesh->mMaterialIndex >= 0)
    {
      aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
      
      // Intentar cargar textura difusa (aiTextureType_DIFFUSE = 1)
      unsigned int texture_id = loadMaterialTexture(material, scene, directory, 1);
      
      if (texture_id > 0)
      {
        processed_mesh->setTexture(texture_id);
        std::cout << "  Texture loaded for mesh (ID: " << texture_id << ")" << std::endl;
      }
    }

    return processed_mesh;
  }

  unsigned int AssimpLoader::loadMaterialTexture(
      aiMaterial *mat,
      const aiScene *scene,
      const std::string &directory,
      int textureType)
  {
    // Verificar si hay texturas de este tipo
    unsigned int texture_count = mat->GetTextureCount((aiTextureType)textureType);
    if (texture_count == 0)
    {
      return 0;
    }

    // Obtener la primera textura
    aiString str;
    mat->GetTexture((aiTextureType)textureType, 0, &str);
    std::string texture_filename = str.C_Str();

    // Verificar si es una textura embebida (comienza con *)
    if (texture_filename[0] == '*')
    {
      // Extraer el índice de la textura embebida
      int texture_index = std::atoi(&texture_filename[1]);
      
      if (texture_index < 0 || texture_index >= (int)scene->mNumTextures)
      {
        std::cerr << "Invalid embedded texture index: " << texture_index << std::endl;
        return 0;
      }

      const aiTexture *embedded_texture = scene->mTextures[texture_index];
      return loadEmbeddedTexture(embedded_texture);
    }
    else
    {
      // Es una textura externa - usar TextureManager
      std::string full_path = directory + "/" + texture_filename;
      auto *texture = Graphics::Textures::TextureManager::getInstance().getTexture(full_path);
      
      if (!texture)
      {
        // Intentar cargar la textura
        if (Graphics::Textures::TextureManager::getInstance().loadTexture2D(full_path, full_path, true))
        {
          texture = Graphics::Textures::TextureManager::getInstance().getTexture(full_path);
        }
      }
      
      return texture ? texture->getId() : 0;
    }

    return 0;
  }

  unsigned int AssimpLoader::loadEmbeddedTexture(const aiTexture *texture)
  {
    unsigned int texture_id;
    glGenTextures(1, &texture_id);

    if (texture->mHeight == 0)
    {
      // Textura comprimida (PNG, JPG, etc.) - necesita decodificación
      int width, height, channels;
      unsigned char *data = stbi_load_from_memory(
          reinterpret_cast<const unsigned char *>(texture->pcData),
          texture->mWidth,
          &width, &height, &channels, 0);

      if (data)
      {
        GLenum format = GL_RGB;
        if (channels == 1)
          format = GL_RED;
        else if (channels == 3)
          format = GL_RGB;
        else if (channels == 4)
          format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, texture_id);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
        std::cout << "  Embedded texture loaded: " << width << "x" << height << " (" << channels << " channels)" << std::endl;
      }
      else
      {
        std::cerr << "Failed to load embedded texture from memory" << std::endl;
        glDeleteTextures(1, &texture_id);
        return 0;
      }
    }
    else
    {
      // Textura sin comprimir (datos ARGB8888 raw)
      glBindTexture(GL_TEXTURE_2D, texture_id);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture->mWidth, texture->mHeight, 0, GL_BGRA, GL_UNSIGNED_BYTE, texture->pcData);
      glGenerateMipmap(GL_TEXTURE_2D);

      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

      std::cout << "  Embedded raw texture loaded: " << texture->mWidth << "x" << texture->mHeight << std::endl;
    }

    return texture_id;
  }

} // namespace Utils
