#ifndef MESH_H
#define MESH_H

#include "buffer_objects.h"
#include "../shaders/shader_manager.h"

#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include <string>

namespace Graphics
{
    namespace Rendering
    {

        // Estructura para vértice completo
        struct Vertex
        {
            glm::vec3 position;
            glm::vec3 normal;
            glm::vec2 texture_coords;
            glm::vec3 tangent;
            glm::vec3 bitangent;
            glm::vec3 color; // Color del vértice (para materiales)

            Vertex() = default;
            Vertex(const glm::vec3 &pos)
                : position(pos), normal(0.0f, 1.0f, 0.0f), texture_coords(0.0f, 0.0f),
                  tangent(1.0f, 0.0f, 0.0f), bitangent(0.0f, 0.0f, 1.0f), color(1.0f, 1.0f, 1.0f) {}

            Vertex(const glm::vec3 &pos, const glm::vec3 &norm, const glm::vec2 &tex)
                : position(pos), normal(norm), texture_coords(tex),
                  tangent(1.0f, 0.0f, 0.0f), bitangent(0.0f, 0.0f, 1.0f), color(1.0f, 1.0f, 1.0f) {}
        };

        // Estructura para datos de instancia
        struct InstanceAttributes
        {
            glm::vec3 instance_position;
            glm::vec3 instance_scale;
            float instance_rotation_y;
            float instance_billboard;
        };

        // Clase Mesh modernizada
        class Mesh
        {
        private:
            std::vector<Vertex> vertices_;
            std::vector<unsigned int> indices_;
            std::unique_ptr<VertexArray> vao_;
            std::unique_ptr<VertexBuffer> vbo_;
            std::unique_ptr<IndexBuffer> ebo_;

            // Textura del mesh
            unsigned int texture_id_;
            bool has_texture_;

            std::string name_;
            bool initialized_;

            void setupMesh();
            void calculateTangents();

        public:
            Mesh();
            Mesh(const std::string &name);
            Mesh(const std::vector<Vertex> &vertices, const std::vector<unsigned int> &indices, const std::string &name = "");
            ~Mesh() = default;

            // No permitir copia
            Mesh(const Mesh &) = delete;
            Mesh &operator=(const Mesh &) = delete;

            // Permitir movimiento
            Mesh(Mesh &&other) noexcept;
            Mesh &operator=(Mesh &&other) noexcept;

            // Renderizado
            void draw() const;

            // Textura
            bool hasTexture() const { return has_texture_; }
        };

        // Factory para crear meshes comunes

    } // namespace Rendering
} // namespace Graphics

#endif // MESH_H