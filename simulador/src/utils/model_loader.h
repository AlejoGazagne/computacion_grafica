#ifndef MODEL_LOADER_H
#define MODEL_LOADER_H

#include <string>
#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include "../scene/model.h"

// Forward declarations de Assimp (en el namespace global)
struct aiNode;
struct aiScene;
struct aiMesh;

namespace Utils
{

  /**
   * @brief Clase para cargar modelos 3D usando Assimp
   */
  class ModelLoader
  {
  public:
    /**
     * @brief Carga un modelo 3D desde un archivo
     * @param filepath Ruta al archivo del modelo (ej: .obj, .fbx, .dae, etc.)
     * @param model_name Nombre opcional para el modelo
     * @return unique_ptr al modelo cargado, o nullptr si falla
     */
    static std::unique_ptr<Scene::Model> loadModel(const std::string &filepath, 
                                                    const std::string &model_name = "");

  private:
    /**
     * @brief Procesa un nodo de la jerarquía de Assimp recursivamente
     */
    static void processNode(::aiNode *node, 
                           const ::aiScene *scene,
                           std::vector<std::unique_ptr<Graphics::Rendering::Mesh>> &meshes,
                           const std::string &directory,
                           const glm::vec3 &center_offset);

    /**
     * @brief Procesa un mesh individual de Assimp
     */
    static std::unique_ptr<Graphics::Rendering::Mesh> processMesh(::aiMesh *mesh, 
                                                                   const ::aiScene *scene,
                                                                   const std::string &directory,
                                                                   const glm::vec3 &center_offset);

    /**
     * @brief Calcula el centro geométrico del modelo
     */
    static glm::vec3 calculateModelCenter(const ::aiScene *scene);

    /**
     * @brief Extrae el directorio de una ruta de archivo
     */
    static std::string getDirectory(const std::string &filepath);
  };

} // namespace Utils

#endif // MODEL_LOADER_H
