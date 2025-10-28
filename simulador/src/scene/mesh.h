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

        // Estructura para datos de instancia (para instancing)
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

            // Instancing
            std::unique_ptr<VertexBuffer> instance_vbo_;
            unsigned int instance_count_;

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

            // Configurar datos de mesh
            void setVertices(const std::vector<Vertex> &vertices);
            void setIndices(const std::vector<unsigned int> &indices);
            void setData(const std::vector<Vertex> &vertices, const std::vector<unsigned int> &indices);

            // Actualizar datos (para meshes dinámicos)
            void updateVertices(const std::vector<Vertex> &vertices);
            void updateIndices(const std::vector<unsigned int> &indices);

            // Renderizado
            void draw() const;
            void drawInstanced(unsigned int count) const;

            // Instancing support
            void setInstanceData(const std::vector<InstanceAttributes> &instance_data);
            unsigned int getInstanceCount() const { return instance_count_; }
            bool hasInstanceData() const { return instance_count_ > 0; }

            // Getters
            const std::vector<Vertex> &getVertices() const { return vertices_; }
            const std::vector<unsigned int> &getIndices() const { return indices_; }
            const std::string &getName() const { return name_; }
            bool isInitialized() const { return initialized_; }
            size_t getVertexCount() const { return vertices_.size(); }
            size_t getTriangleCount() const { return indices_.size() / 3; }

            // Textura
            void setTexture(unsigned int texture_id) { texture_id_ = texture_id; has_texture_ = true; }
            unsigned int getTextureID() const { return texture_id_; }
            bool hasTexture() const { return has_texture_; }

            // Utilidades
            void calculateNormals();
            void recalculateTangents() { calculateTangents(); }

            // Transformaciones de vértices
            void transform(const glm::mat4 &matrix);
            void translate(const glm::vec3 &translation);
            void scale(const glm::vec3 &scale);
            void rotate(float angle, const glm::vec3 &axis);

            // Información de bounding
            glm::vec3 getMinBounds() const;
            glm::vec3 getMaxBounds() const;
            glm::vec3 getCenter() const;
            float getBoundingRadius() const;
        };

        // Factory para crear meshes comunes
        class MeshFactory
        {
        public:
            static std::unique_ptr<Mesh> createCube(float size = 1.0f, const std::string &name = "cube");
            static std::unique_ptr<Mesh> createSphere(float radius = 1.0f, unsigned int rings = 32, unsigned int sectors = 32, const std::string &name = "sphere");
            static std::unique_ptr<Mesh> createPlane(float width = 1.0f, float height = 1.0f, const std::string &name = "plane");
            static std::unique_ptr<Mesh> createCylinder(float radius = 1.0f, float height = 2.0f, unsigned int segments = 32, const std::string &name = "cylinder");
            static std::unique_ptr<Mesh> createCone(float radius = 1.0f, float height = 2.0f, unsigned int segments = 32, const std::string &name = "cone");

            // Mesh para skybox (cubo invertido)
            static std::unique_ptr<Mesh> createSkyboxCube(const std::string &name = "skybox");

            // Mesh para quad de pantalla completa
            static std::unique_ptr<Mesh> createScreenQuad(const std::string &name = "screen_quad");
        };

        // Utilidades de geometría
        namespace GeometryUtils
        {
            // Generar normales para un conjunto de vértices
            void generateNormals(std::vector<Vertex> &vertices, const std::vector<unsigned int> &indices);

            // Generar coordenadas de textura esféricas
            void generateSphericalTexCoords(std::vector<Vertex> &vertices);

            // Calcular tangentes y bitangentes
            void calculateTangentSpace(std::vector<Vertex> &vertices, const std::vector<unsigned int> &indices);

            // Subdividir triángulos (para mayor detalle)
            void subdivideTriangles(std::vector<Vertex> &vertices, std::vector<unsigned int> &indices);
        }

    } // namespace Rendering
} // namespace Graphics

#endif // MESH_H