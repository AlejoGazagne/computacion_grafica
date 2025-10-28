// NEW FILE: src/utils/assimp_loader.h
#ifndef ASSIMP_LOADER_H
#define ASSIMP_LOADER_H

#include "../scene/mesh.h"
#include "../scene/model.h"
#include "../graphics/textures/texture_manager.h"
#include <glm/glm.hpp>
#include <string>
#include <memory>
#include <map>

// Forward declarations de Assimp
struct aiNode;
struct aiScene;
struct aiMesh;
struct aiMaterial;
struct aiTexture;

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
        const glm::vec3 &uniformColor,
        const std::string &directory);

    /**
     * @brief Procesa una malla de Assimp
     */
    static std::unique_ptr<Graphics::Rendering::Mesh> processMesh(
        aiMesh *mesh,
        const aiScene *scene,
        const glm::vec3 &uniformColor,
        const std::string &directory);

    /**
     * @brief Carga una textura desde un material
     * @return ID de la textura de OpenGL, o 0 si falla
     */
    static unsigned int loadMaterialTexture(
        aiMaterial *mat,
        const aiScene *scene,
        const std::string &directory,
        int textureType);

    /**
     * @brief Carga una textura embebida desde memoria
     */
    static unsigned int loadEmbeddedTexture(
        const aiTexture *texture);
  };

} // namespace Utils

#endif // ASSIMP_LOADER_H
