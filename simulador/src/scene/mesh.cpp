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

        Mesh::Mesh() : texture_id_(0), has_texture_(false), name_("unnamed_mesh"), initialized_(false)
        {
        }

        Mesh::Mesh(const std::string &name) : texture_id_(0), has_texture_(false), name_(name), initialized_(false)
        {
        }

        Mesh::Mesh(const std::vector<Vertex> &vertices, const std::vector<unsigned int> &indices, const std::string &name)
            : vertices_(vertices), indices_(indices), texture_id_(0), has_texture_(false), name_(name.empty() ? "unnamed_mesh" : name), initialized_(false)
        {
            setupMesh();
        }

        Mesh::Mesh(Mesh &&other) noexcept
            : vertices_(std::move(other.vertices_)), indices_(std::move(other.indices_)),
              vao_(std::move(other.vao_)), vbo_(std::move(other.vbo_)), ebo_(std::move(other.ebo_)),
              texture_id_(other.texture_id_), has_texture_(other.has_texture_),
              name_(std::move(other.name_)), initialized_(other.initialized_)
        {
            other.initialized_ = false;
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
                name_ = std::move(other.name_);
                initialized_ = other.initialized_;
                texture_id_ = other.texture_id_;
                has_texture_ = other.has_texture_;
                other.initialized_ = false;
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

        // void Mesh::drawInstanced(unsigned int count) const
        // {
        //     if (!initialized_)
        //         return;

        //     vao_->bind();

        //     if (vao_->hasIndexBuffer())
        //     {
        //         glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(indices_.size()), GL_UNSIGNED_INT, 0, count);
        //     }
        //     else
        //     {
        //         glDrawArraysInstanced(GL_TRIANGLES, 0, static_cast<GLsizei>(vertices_.size()), count);
        //     }

        //     vao_->unbind();
        // }

    } // namespace Rendering
} // namespace Graphics