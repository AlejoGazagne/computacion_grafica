/**
 * @file waypoint_renderer.h
 * @brief Renderiza waypoints como marcadores visuales 3D en el mundo
 */

#pragma once

#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "graphics/rendering/buffer_objects.h"

extern "C"
{
#include <glad/glad.h>
}

namespace Graphics
{
  namespace Shaders
  {
    class Shader;
  }
}

namespace Mission
{
  /**
   * @brief Renderiza waypoints como marcadores visuales 3D (cilindros).
   */
  class WaypointRenderer
  {
  public:
    WaypointRenderer() = default;
    ~WaypointRenderer();

    /**
     * @brief Compila el shader y crea la geometría del cilindro.
     */
    void initialize();

    /**
     * @brief Limpia recursos OpenGL.
     */
    void cleanup();

    /**
     * @brief Dibuja un waypoint usando matrices de vista/proyección.
     */
    void drawWaypoint(const glm::mat4 &view, const glm::mat4 &proj,
                      const glm::vec3 &position, const glm::vec4 &color,
                      bool isActive = false);

  private:
    std::unique_ptr<Graphics::Rendering::VertexArray> vertex_array_;
    Graphics::Shaders::Shader *shader_ = nullptr;
    int indexCount_ = 0;

    void createCylinderGeometry();
  };

} // namespace Mission
