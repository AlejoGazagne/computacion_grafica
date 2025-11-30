/**
 * @file waypoint_renderer.cpp
 * @brief Implementación del renderizador de waypoints
 */

#include "waypoint_renderer.h"
#include "graphics/shaders/shader_manager.h"
#include <cmath>
#include <vector>
#include <iostream>

namespace Mission
{

  WaypointRenderer::~WaypointRenderer()
  {
    cleanup();
  }

  void WaypointRenderer::initialize()
  {
    // Cargar shader especializado
    auto &shader_manager = Graphics::Shaders::ShaderManager::getInstance();
    shader_manager.loadShader("waypoint_shader", "shaders/waypoint.vert", "shaders/waypoint.frag");
    shader_ = shader_manager.getShader("waypoint_shader");

    // Construir la geometría del cilindro
    createCylinderGeometry();

    std::cout << "WaypointRenderer initialized successfully" << std::endl;
  }
  void WaypointRenderer::cleanup()
  {
    vertex_array_.reset();
    shader_ = nullptr;
  }

  void WaypointRenderer::createCylinderGeometry()
  {
    const int segments = 16;
    const float radius = 3.0f;
    const float height = 30.0f;

    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    // Generar vértices del cilindro (pares inferior/superior con normal)
    for (int i = 0; i <= segments; ++i)
    {
      float angle = static_cast<float>(i) / static_cast<float>(segments) * 2.0f * M_PI;
      float x = radius * cosf(angle);
      float z = radius * sinf(angle);

      // Vértice inferior
      vertices.push_back(x);
      vertices.push_back(0.0f);
      vertices.push_back(z);
      // Normal
      vertices.push_back(x / radius);
      vertices.push_back(0.0f);
      vertices.push_back(z / radius);

      // Vértice superior
      vertices.push_back(x);
      vertices.push_back(height);
      vertices.push_back(z);
      // Normal
      vertices.push_back(x / radius);
      vertices.push_back(0.0f);
      vertices.push_back(z / radius);
    }

    // Construir índices para cada quad lateral (2 triángulos por segmento)
    for (int i = 0; i < segments; ++i)
    {
      int base = i * 2;
      // Triángulo 1
      indices.push_back(base);
      indices.push_back(base + 1);
      indices.push_back(base + 2);
      // Triángulo 2
      indices.push_back(base + 1);
      indices.push_back(base + 3);
      indices.push_back(base + 2);
    }

    indexCount_ = static_cast<int>(indices.size());

    // Usar abstracción de buffers
    vertex_array_ = std::make_unique<Graphics::Rendering::VertexArray>();

    auto vbo = std::make_unique<Graphics::Rendering::VertexBuffer>(Graphics::Rendering::BufferUsage::STATIC_DRAW);
    vbo->setData(vertices);

    auto ibo = std::make_unique<Graphics::Rendering::IndexBuffer>(Graphics::Rendering::BufferUsage::STATIC_DRAW);
    ibo->setIndices(indices);

    vertex_array_->addVertexBuffer(std::move(vbo));
    vertex_array_->setIndexBuffer(std::move(ibo));

    // Attribute 0: posición (vec3)
    vertex_array_->addFloatAttribute(0, 3, 6 * sizeof(float), (void *)0);

    // Attribute 1: normal (vec3)
    vertex_array_->addFloatAttribute(1, 3, 6 * sizeof(float), (void *)(3 * sizeof(float)));
  }

  void WaypointRenderer::drawWaypoint(const glm::mat4 &view, const glm::mat4 &proj,
                                      const glm::vec3 &position, const glm::vec4 &color,
                                      bool isActive)
  {
    if (!shader_)
      return;

    shader_->use();

    // Trasladar el cilindro unitario a la posición del waypoint
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);

    shader_->setMat4("model", model);
    shader_->setMat4("view", view);
    shader_->setMat4("projection", proj);
    shader_->setVec3("waypointColor", glm::vec3(color));
    shader_->setFloat("waypointAlpha", color.a);
    shader_->setBool("isActive", isActive);

    if (vertex_array_)
    {
        vertex_array_->bind();
        glDrawElements(GL_TRIANGLES, indexCount_, GL_UNSIGNED_INT, 0);
        vertex_array_->unbind();
    }
  }

} // namespace Mission
