#include "mesh.h"
#include <iostream>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>

namespace Graphics
{
    namespace Rendering
    {

        // === Implementación de Mesh ===

        Mesh::Mesh() : name_("unnamed_mesh"), initialized_(false), instance_count_(0), texture_id_(0), has_texture_(false)
        {
        }

        Mesh::Mesh(const std::string &name) : name_(name), initialized_(false), instance_count_(0), texture_id_(0), has_texture_(false)
        {
        }

        Mesh::Mesh(const std::vector<Vertex> &vertices, const std::vector<unsigned int> &indices, const std::string &name)
            : vertices_(vertices), indices_(indices), name_(name.empty() ? "unnamed_mesh" : name), initialized_(false), instance_count_(0), texture_id_(0), has_texture_(false)
        {
            setupMesh();
        }

        Mesh::Mesh(Mesh &&other) noexcept
            : vertices_(std::move(other.vertices_)), indices_(std::move(other.indices_)),
              vao_(std::move(other.vao_)), vbo_(std::move(other.vbo_)), ebo_(std::move(other.ebo_)),
              instance_vbo_(std::move(other.instance_vbo_)), instance_count_(other.instance_count_),
              name_(std::move(other.name_)), initialized_(other.initialized_),
              texture_id_(other.texture_id_), has_texture_(other.has_texture_)
        {
            other.initialized_ = false;
            other.instance_count_ = 0;
            other.texture_id_ = 0;
            other.has_texture_ = false;
        }

        Mesh &Mesh::operator=(Mesh &&other) noexcept
        {
            if (this != &other)
            {
                vertices_ = std::move(other.vertices_);
                indices_ = std::move(other.indices_);
                vao_ = std::move(other.vao_);
                vbo_ = std::move(other.vbo_);
                ebo_ = std::move(other.ebo_);
                instance_vbo_ = std::move(other.instance_vbo_);
                instance_count_ = other.instance_count_;
                name_ = std::move(other.name_);
                initialized_ = other.initialized_;
                texture_id_ = other.texture_id_;
                has_texture_ = other.has_texture_;
                other.initialized_ = false;
                other.instance_count_ = 0;
                other.texture_id_ = 0;
                other.has_texture_ = false;
            }
            return *this;
        }

        void Mesh::setupMesh()
        {
            if (vertices_.empty())
            {
                std::cerr << "ERROR: Cannot setup mesh '" << name_ << "' with no vertices" << std::endl;
                return;
            }

            // Crear VAO
            vao_ = std::make_unique<VertexArray>();
            vao_->bind();

            // Crear y configurar VBO
            vbo_ = std::make_unique<VertexBuffer>();
            vbo_->setData(vertices_);
            vao_->addVertexBuffer(std::move(vbo_));

            // Configurar atributos de vértice
            // Posición (location = 0)
            vao_->addFloatAttribute(0, 3, sizeof(Vertex), (void *)offsetof(Vertex, position));

            // Normal (location = 1)
            vao_->addFloatAttribute(1, 3, sizeof(Vertex), (void *)offsetof(Vertex, normal));

            // Coordenadas de textura (location = 2)
            vao_->addFloatAttribute(2, 2, sizeof(Vertex), (void *)offsetof(Vertex, texture_coords));

            // Tangente (location = 3)
            vao_->addFloatAttribute(3, 3, sizeof(Vertex), (void *)offsetof(Vertex, tangent));

            // Bitangente (location = 4)
            vao_->addFloatAttribute(4, 3, sizeof(Vertex), (void *)offsetof(Vertex, bitangent));

            // Color del vértice (location = 5)
            vao_->addFloatAttribute(5, 3, sizeof(Vertex), (void *)offsetof(Vertex, color));

            // Configurar EBO si hay índices
            if (!indices_.empty())
            {
                ebo_ = std::make_unique<IndexBuffer>();
                ebo_->setIndices(indices_);
                vao_->setIndexBuffer(std::move(ebo_));
            }

            vao_->unbind();

            initialized_ = true;

            std::cout << "Mesh '" << name_ << "' initialized successfully ("
                      << vertices_.size() << " vertices, "
                      << indices_.size() << " indices)" << std::endl;
        }

        void Mesh::calculateTangents()
        {
            if (indices_.empty() || vertices_.empty())
                return;

            // Reset tangents and bitangents
            for (auto &vertex : vertices_)
            {
                vertex.tangent = glm::vec3(0.0f);
                vertex.bitangent = glm::vec3(0.0f);
            }

            // Calculate tangents for each triangle
            for (size_t i = 0; i < indices_.size(); i += 3)
            {
                unsigned int i0 = indices_[i];
                unsigned int i1 = indices_[i + 1];
                unsigned int i2 = indices_[i + 2];

                Vertex &v0 = vertices_[i0];
                Vertex &v1 = vertices_[i1];
                Vertex &v2 = vertices_[i2];

                glm::vec3 edge1 = v1.position - v0.position;
                glm::vec3 edge2 = v2.position - v0.position;

                glm::vec2 deltaUV1 = v1.texture_coords - v0.texture_coords;
                glm::vec2 deltaUV2 = v2.texture_coords - v0.texture_coords;

                float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

                glm::vec3 tangent;
                tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
                tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
                tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

                glm::vec3 bitangent;
                bitangent.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
                bitangent.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
                bitangent.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);

                v0.tangent += tangent;
                v1.tangent += tangent;
                v2.tangent += tangent;

                v0.bitangent += bitangent;
                v1.bitangent += bitangent;
                v2.bitangent += bitangent;
            }

            // Normalize tangents and bitangents
            for (auto &vertex : vertices_)
            {
                vertex.tangent = glm::normalize(vertex.tangent);
                vertex.bitangent = glm::normalize(vertex.bitangent);
            }
        }

        void Mesh::setVertices(const std::vector<Vertex> &vertices)
        {
            vertices_ = vertices;
            if (initialized_)
            {
                setupMesh();
            }
        }

        void Mesh::setIndices(const std::vector<unsigned int> &indices)
        {
            indices_ = indices;
            if (initialized_)
            {
                setupMesh();
            }
        }

        void Mesh::setData(const std::vector<Vertex> &vertices, const std::vector<unsigned int> &indices)
        {
            vertices_ = vertices;
            indices_ = indices;
            setupMesh();
        }

        void Mesh::updateVertices(const std::vector<Vertex> &vertices)
        {
            if (!initialized_)
                return;

            vertices_ = vertices;
            vao_->bind();
            if (vbo_)
            {
                vbo_->updateData(vertices_);
            }
            vao_->unbind();
        }

        void Mesh::updateIndices(const std::vector<unsigned int> &indices)
        {
            if (!initialized_)
                return;

            indices_ = indices;
            vao_->bind();
            if (ebo_)
            {
                ebo_->setIndices(indices_);
            }
            vao_->unbind();
        }

        void Mesh::draw() const
        {
            if (!initialized_)
                return;

            // Activar textura si está disponible
            if (has_texture_)
            {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, texture_id_);
            }

            vao_->bind();

            if (vao_->hasIndexBuffer())
            {
                glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices_.size()), GL_UNSIGNED_INT, 0);
            }
            else
            {
                glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertices_.size()));
            }

            vao_->unbind();
            
            // Desactivar textura
            if (has_texture_)
            {
                glBindTexture(GL_TEXTURE_2D, 0);
            }
        }

        void Mesh::drawInstanced(unsigned int count) const
        {
            if (!initialized_)
                return;

            vao_->bind();

            if (vao_->hasIndexBuffer())
            {
                glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(indices_.size()), GL_UNSIGNED_INT, 0, count);
            }
            else
            {
                glDrawArraysInstanced(GL_TRIANGLES, 0, static_cast<GLsizei>(vertices_.size()), count);
            }

            vao_->unbind();
        }

        void Mesh::calculateNormals()
        {
            if (indices_.empty() || vertices_.empty())
                return;

            // Reset normals
            for (auto &vertex : vertices_)
            {
                vertex.normal = glm::vec3(0.0f);
            }

            // Calculate face normals and accumulate
            for (size_t i = 0; i < indices_.size(); i += 3)
            {
                unsigned int i0 = indices_[i];
                unsigned int i1 = indices_[i + 1];
                unsigned int i2 = indices_[i + 2];

                glm::vec3 v0 = vertices_[i0].position;
                glm::vec3 v1 = vertices_[i1].position;
                glm::vec3 v2 = vertices_[i2].position;

                glm::vec3 normal = glm::normalize(glm::cross(v1 - v0, v2 - v0));

                vertices_[i0].normal += normal;
                vertices_[i1].normal += normal;
                vertices_[i2].normal += normal;
            }

            // Normalize accumulated normals
            for (auto &vertex : vertices_)
            {
                vertex.normal = glm::normalize(vertex.normal);
            }

            // Update VBO if mesh is initialized
            if (initialized_)
            {
                updateVertices(vertices_);
            }
        }

        void Mesh::transform(const glm::mat4 &matrix)
        {
            glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(matrix)));

            for (auto &vertex : vertices_)
            {
                vertex.position = glm::vec3(matrix * glm::vec4(vertex.position, 1.0f));
                vertex.normal = glm::normalize(normalMatrix * vertex.normal);
                vertex.tangent = glm::normalize(normalMatrix * vertex.tangent);
                vertex.bitangent = glm::normalize(normalMatrix * vertex.bitangent);
            }

            if (initialized_)
            {
                updateVertices(vertices_);
            }
        }

        void Mesh::translate(const glm::vec3 &translation)
        {
            transform(glm::translate(glm::mat4(1.0f), translation));
        }

        void Mesh::scale(const glm::vec3 &scale)
        {
            transform(glm::scale(glm::mat4(1.0f), scale));
        }

        void Mesh::rotate(float angle, const glm::vec3 &axis)
        {
            transform(glm::rotate(glm::mat4(1.0f), angle, axis));
        }

        glm::vec3 Mesh::getMinBounds() const
        {
            if (vertices_.empty())
                return glm::vec3(0.0f);

            glm::vec3 min = vertices_[0].position;
            for (const auto &vertex : vertices_)
            {
                min = glm::min(min, vertex.position);
            }
            return min;
        }

        glm::vec3 Mesh::getMaxBounds() const
        {
            if (vertices_.empty())
                return glm::vec3(0.0f);

            glm::vec3 max = vertices_[0].position;
            for (const auto &vertex : vertices_)
            {
                max = glm::max(max, vertex.position);
            }
            return max;
        }

        glm::vec3 Mesh::getCenter() const
        {
            return (getMinBounds() + getMaxBounds()) * 0.5f;
        }

        float Mesh::getBoundingRadius() const
        {
            glm::vec3 center = getCenter();
            float maxDistance = 0.0f;

            for (const auto &vertex : vertices_)
            {
                float distance = glm::length(vertex.position - center);
                maxDistance = std::max(maxDistance, distance);
            }

            return maxDistance;
        }

        // === Implementación de MeshFactory ===

        std::unique_ptr<Mesh> MeshFactory::createCube(float size, const std::string &name)
        {
            float half = size * 0.5f;

            std::vector<Vertex> vertices = {
                // Front face
                {{-half, -half, half}, {0, 0, 1}, {0, 0}},
                {{half, -half, half}, {0, 0, 1}, {1, 0}},
                {{half, half, half}, {0, 0, 1}, {1, 1}},
                {{-half, half, half}, {0, 0, 1}, {0, 1}},

                // Back face
                {{half, -half, -half}, {0, 0, -1}, {0, 0}},
                {{-half, -half, -half}, {0, 0, -1}, {1, 0}},
                {{-half, half, -half}, {0, 0, -1}, {1, 1}},
                {{half, half, -half}, {0, 0, -1}, {0, 1}},

                // Left face
                {{-half, -half, -half}, {-1, 0, 0}, {0, 0}},
                {{-half, -half, half}, {-1, 0, 0}, {1, 0}},
                {{-half, half, half}, {-1, 0, 0}, {1, 1}},
                {{-half, half, -half}, {-1, 0, 0}, {0, 1}},

                // Right face
                {{half, -half, half}, {1, 0, 0}, {0, 0}},
                {{half, -half, -half}, {1, 0, 0}, {1, 0}},
                {{half, half, -half}, {1, 0, 0}, {1, 1}},
                {{half, half, half}, {1, 0, 0}, {0, 1}},

                // Top face
                {{-half, half, half}, {0, 1, 0}, {0, 0}},
                {{half, half, half}, {0, 1, 0}, {1, 0}},
                {{half, half, -half}, {0, 1, 0}, {1, 1}},
                {{-half, half, -half}, {0, 1, 0}, {0, 1}},

                // Bottom face
                {{-half, -half, -half}, {0, -1, 0}, {0, 0}},
                {{half, -half, -half}, {0, -1, 0}, {1, 0}},
                {{half, -half, half}, {0, -1, 0}, {1, 1}},
                {{-half, -half, half}, {0, -1, 0}, {0, 1}}};

            std::vector<unsigned int> indices = {
                0, 1, 2, 2, 3, 0,       // Front
                4, 5, 6, 6, 7, 4,       // Back
                8, 9, 10, 10, 11, 8,    // Left
                12, 13, 14, 14, 15, 12, // Right
                16, 17, 18, 18, 19, 16, // Top
                20, 21, 22, 22, 23, 20  // Bottom
            };

            auto mesh = std::make_unique<Mesh>(name);
            mesh->setData(vertices, indices);
            return mesh;
        }

        std::unique_ptr<Mesh> MeshFactory::createSphere(float radius, unsigned int rings, unsigned int sectors, const std::string &name)
        {
            std::vector<Vertex> vertices;
            std::vector<unsigned int> indices;

            float const R = 1.0f / (float)(rings - 1);
            float const S = 1.0f / (float)(sectors - 1);

            for (unsigned int r = 0; r < rings; ++r)
            {
                for (unsigned int s = 0; s < sectors; ++s)
                {
                    float const y = sin(-M_PI_2 + M_PI * r * R);
                    float const x = cos(2 * M_PI * s * S) * sin(M_PI * r * R);
                    float const z = sin(2 * M_PI * s * S) * sin(M_PI * r * R);

                    Vertex vertex;
                    vertex.position = glm::vec3(x, y, z) * radius;
                    vertex.normal = glm::vec3(x, y, z);
                    vertex.texture_coords = glm::vec2(s * S, r * R);

                    vertices.push_back(vertex);
                }
            }

            for (unsigned int r = 0; r < rings - 1; ++r)
            {
                for (unsigned int s = 0; s < sectors - 1; ++s)
                {
                    unsigned int curRow = r * sectors;
                    unsigned int nextRow = (r + 1) * sectors;

                    indices.push_back(curRow + s);
                    indices.push_back(nextRow + s);
                    indices.push_back(nextRow + (s + 1));

                    indices.push_back(curRow + s);
                    indices.push_back(nextRow + (s + 1));
                    indices.push_back(curRow + (s + 1));
                }
            }

            auto mesh = std::make_unique<Mesh>(name);
            mesh->setData(vertices, indices);
            return mesh;
        }

        std::unique_ptr<Mesh> MeshFactory::createPlane(float width, float height, const std::string &name)
        {
            float halfWidth = width * 0.5f;
            float halfHeight = height * 0.5f;

            std::vector<Vertex> vertices = {
                {{-halfWidth, 0, -halfHeight}, {0, 1, 0}, {0, 0}},
                {{halfWidth, 0, -halfHeight}, {0, 1, 0}, {1, 0}},
                {{halfWidth, 0, halfHeight}, {0, 1, 0}, {1, 1}},
                {{-halfWidth, 0, halfHeight}, {0, 1, 0}, {0, 1}}};

            std::vector<unsigned int> indices = {
                0, 1, 2, 2, 3, 0};

            auto mesh = std::make_unique<Mesh>(name);
            mesh->setData(vertices, indices);
            return mesh;
        }

        std::unique_ptr<Mesh> MeshFactory::createSkyboxCube(const std::string &name)
        {
            float size = 1.0f;

            std::vector<Vertex> vertices = {
                // positions
                {{-size, size, -size}, {0, 0, 0}, {0, 0}},
                {{-size, -size, -size}, {0, 0, 0}, {0, 0}},
                {{size, -size, -size}, {0, 0, 0}, {0, 0}},
                {{size, -size, -size}, {0, 0, 0}, {0, 0}},
                {{size, size, -size}, {0, 0, 0}, {0, 0}},
                {{-size, size, -size}, {0, 0, 0}, {0, 0}},

                {{-size, -size, size}, {0, 0, 0}, {0, 0}},
                {{-size, -size, -size}, {0, 0, 0}, {0, 0}},
                {{-size, size, -size}, {0, 0, 0}, {0, 0}},
                {{-size, size, -size}, {0, 0, 0}, {0, 0}},
                {{-size, size, size}, {0, 0, 0}, {0, 0}},
                {{-size, -size, size}, {0, 0, 0}, {0, 0}},

                {{size, -size, -size}, {0, 0, 0}, {0, 0}},
                {{size, -size, size}, {0, 0, 0}, {0, 0}},
                {{size, size, size}, {0, 0, 0}, {0, 0}},
                {{size, size, size}, {0, 0, 0}, {0, 0}},
                {{size, size, -size}, {0, 0, 0}, {0, 0}},
                {{size, -size, -size}, {0, 0, 0}, {0, 0}},

                {{-size, -size, size}, {0, 0, 0}, {0, 0}},
                {{-size, size, size}, {0, 0, 0}, {0, 0}},
                {{size, size, size}, {0, 0, 0}, {0, 0}},
                {{size, size, size}, {0, 0, 0}, {0, 0}},
                {{size, -size, size}, {0, 0, 0}, {0, 0}},
                {{-size, -size, size}, {0, 0, 0}, {0, 0}},

                {{-size, size, -size}, {0, 0, 0}, {0, 0}},
                {{size, size, -size}, {0, 0, 0}, {0, 0}},
                {{size, size, size}, {0, 0, 0}, {0, 0}},
                {{size, size, size}, {0, 0, 0}, {0, 0}},
                {{-size, size, size}, {0, 0, 0}, {0, 0}},
                {{-size, size, -size}, {0, 0, 0}, {0, 0}},

                {{-size, -size, -size}, {0, 0, 0}, {0, 0}},
                {{-size, -size, size}, {0, 0, 0}, {0, 0}},
                {{size, -size, -size}, {0, 0, 0}, {0, 0}},
                {{size, -size, -size}, {0, 0, 0}, {0, 0}},
                {{-size, -size, size}, {0, 0, 0}, {0, 0}},
                {{size, -size, size}, {0, 0, 0}, {0, 0}}};

            auto mesh = std::make_unique<Mesh>(name);
            mesh->setVertices(vertices);
            return mesh;
        }

        std::unique_ptr<Mesh> MeshFactory::createScreenQuad(const std::string &name)
        {
            std::vector<Vertex> vertices = {
                {{-1.0f, 1.0f, 0.0f}, {0, 0, 1}, {0, 1}},
                {{-1.0f, -1.0f, 0.0f}, {0, 0, 1}, {0, 0}},
                {{1.0f, -1.0f, 0.0f}, {0, 0, 1}, {1, 0}},
                {{1.0f, 1.0f, 0.0f}, {0, 0, 1}, {1, 1}}};

            std::vector<unsigned int> indices = {
                0, 1, 2, 2, 3, 0};

            auto mesh = std::make_unique<Mesh>(name);
            mesh->setData(vertices, indices);
            return mesh;
        }

        void Mesh::setInstanceData(const std::vector<InstanceAttributes> &instance_data)
        {
            if (!initialized_ || !vao_)
            {
                std::cerr << "ERROR: Cannot set instance data on uninitialized mesh" << std::endl;
                return;
            }

            if (instance_data.empty())
            {
                instance_count_ = 0;
                instance_vbo_.reset();
                return;
            }

            instance_count_ = static_cast<unsigned int>(instance_data.size());

            // Crear VBO para instancias
            instance_vbo_ = std::make_unique<VertexBuffer>(BufferUsage::DYNAMIC_DRAW);
            instance_vbo_->setData(instance_data);

            // Configurar atributos de instancia en el VAO
            vao_->bind();

            // Atributo 6: posición de instancia (vec3)
            vao_->addFloatAttribute(6, 3, sizeof(InstanceAttributes),
                                    (void *)offsetof(InstanceAttributes, instance_position));
            glVertexAttribDivisor(6, 1);

            // Atributo 7: escala de instancia (vec3)
            vao_->addFloatAttribute(7, 3, sizeof(InstanceAttributes),
                                    (void *)offsetof(InstanceAttributes, instance_scale));
            glVertexAttribDivisor(7, 1);

            // Atributo 8: rotación Y de instancia (float)
            vao_->addFloatAttribute(8, 1, sizeof(InstanceAttributes),
                                    (void *)offsetof(InstanceAttributes, instance_rotation_y));
            glVertexAttribDivisor(8, 1);

            // Atributo 9: billboard flag (float)
            vao_->addFloatAttribute(9, 1, sizeof(InstanceAttributes),
                                    (void *)offsetof(InstanceAttributes, instance_billboard));
            glVertexAttribDivisor(9, 1);

            vao_->unbind();

            std::cout << "Instance data set for mesh '" << name_ << "' ("
                      << instance_count_ << " instances)" << std::endl;
        }

    } // namespace Rendering
} // namespace Graphics