// NEW FILE: src/utils/assimp_loader.h
#ifndef ASSIMP_LOADER_H
#define ASSIMP_LOADER_H

#include "../scene/mesh.h"
#include "../scene/model.h"
#include <glm/glm.hpp>
#include <string>
#include <memory>

// Forward declarations de Assimp
struct aiNode;
struct aiScene;
struct aiMesh;

namespace Utils
{

  /**
   * @class AssimpLoader
   * @brief Carga modelos 3D usando la biblioteca Assimp
   */
  class AssimpLoader
  {
  public:
    /**
     * @brief Carga un modelo 3D desde un archivo usando Assimp
     * @param filepath Ruta al archivo del modelo
     * @param uniformColor Color uniforme a aplicar (opcional, por defecto gris)
     * @return Puntero único al modelo cargado
     */
    static std::unique_ptr<Scene::Model> loadModel(
        const std::string &filepath,
        const glm::vec3 &uniformColor = glm::vec3(0.5f, 0.5f, 0.5f));

  private:
    /**
     * @brief Procesa un nodo de Assimp y todos sus hijos
     */
    static void processNode(
        aiNode *node,
        const aiScene *scene,
        Scene::Model *model,
        const glm::vec3 &uniformColor);

    /**
     * @brief Procesa una malla de Assimp
     */
    static std::unique_ptr<Graphics::Rendering::Mesh> processMesh(
        aiMesh *mesh,
        const aiScene *scene,
        const glm::vec3 &uniformColor);
  };

} // namespace Utils

#endif // ASSIMP_LOADER_H
