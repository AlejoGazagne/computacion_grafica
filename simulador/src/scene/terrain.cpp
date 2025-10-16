#include "terrain.h"
#include "../utils/perlin_noise.h"
#include <iostream>
#include <cmath>
#include <glad/glad.h>

namespace Scene {

    Terrain::Terrain(const std::string& name)
        : VAO_(0), VBO_(0), EBO_(0), name_(name), vertex_count_(0), index_count_(0) {
    }

    Terrain::~Terrain() {
        cleanup();
    }

    Terrain::Terrain(Terrain&& other) noexcept
        : VAO_(other.VAO_), VBO_(other.VBO_), EBO_(other.EBO_),
          config_(std::move(other.config_)), name_(std::move(other.name_)),
          vertices_(std::move(other.vertices_)), indices_(std::move(other.indices_)),
          vertex_count_(other.vertex_count_), index_count_(other.index_count_) {
        
        other.VAO_ = 0;
        other.VBO_ = 0;
        other.EBO_ = 0;
        other.vertex_count_ = 0;
        other.index_count_ = 0;
    }

    Terrain& Terrain::operator=(Terrain&& other) noexcept {
        if (this != &other) {
            cleanup();
            
            VAO_ = other.VAO_;
            VBO_ = other.VBO_;
            EBO_ = other.EBO_;
            config_ = std::move(other.config_);
            name_ = std::move(other.name_);
            vertices_ = std::move(other.vertices_);
            indices_ = std::move(other.indices_);
            vertex_count_ = other.vertex_count_;
            index_count_ = other.index_count_;
            
            other.VAO_ = 0;
            other.VBO_ = 0;
            other.EBO_ = 0;
            other.vertex_count_ = 0;
            other.index_count_ = 0;
        }
        return *this;
    }

    bool Terrain::initialize(const TerrainConfig& config) {
        config_ = config;
        
        // Generar vértices e índices
        generateVertices();
        generateIndices();
        
        // Configurar buffers de OpenGL
        setupBuffers();
        
        std::cout << "Terrain '" << name_ << "' initialized successfully (" 
                  << vertex_count_ << " vertices, " << index_count_ << " indices)" << std::endl;
        
        return true;
    }

    void Terrain::generateVertices() {
        vertices_.clear();
        
        // Crear generador de Perlin Noise si está habilitado
        Utils::PerlinNoise perlin(config_.noise_seed);
        
        // Calcular el paso entre vértices
        float x_step = config_.width / static_cast<float>(config_.width_segments);
        float z_step = config_.depth / static_cast<float>(config_.depth_segments);
        
        // Calcular el paso de coordenadas de textura
        float u_step = config_.texture_repeat / static_cast<float>(config_.width_segments);
        float v_step = config_.texture_repeat / static_cast<float>(config_.depth_segments);
        
        // Calcular posición inicial (centrado en el origen)
        float start_x = -config_.width * 0.5f;
        float start_z = -config_.depth * 0.5f;
        
        // Vector para almacenar las alturas (necesario para calcular normales correctamente)
        std::vector<std::vector<float>> heights(config_.depth_segments + 1, 
                                                 std::vector<float>(config_.width_segments + 1));
        
        // Primera pasada: Calcular alturas con Perlin Noise
        for (int z = 0; z <= config_.depth_segments; ++z) {
            for (int x = 0; x <= config_.width_segments; ++x) {
                float pos_x = start_x + x * x_step;
                float pos_z = start_z + z * z_step;
                
                // Calcular altura usando Perlin Noise
                float height = config_.y_position;
                if (config_.use_perlin_noise) {
                    height = config_.y_position + perlin.getTerrainHeight(
                        pos_x, pos_z, 
                        config_.noise_scale, 
                        config_.height_multiplier, 
                        config_.noise_octaves
                    );
                }
                
                heights[z][x] = height;
            }
        }
        
        // Segunda pasada: Generar vértices con normales calculadas correctamente
        for (int z = 0; z <= config_.depth_segments; ++z) {
            for (int x = 0; x <= config_.width_segments; ++x) {
                // Posición del vértice
                float pos_x = start_x + x * x_step;
                float pos_y = heights[z][x];
                float pos_z = start_z + z * z_step;
                
                // Coordenadas de textura
                float u = x * u_step;
                float v = z * v_step;
                
                // Calcular normal usando diferencias finitas (método del producto cruz)
                glm::vec3 normal(0.0f, 1.0f, 0.0f);
                
                if (config_.use_perlin_noise) {
                    // Obtener alturas de vértices vecinos para calcular la normal
                    float hL = (x > 0) ? heights[z][x - 1] : pos_y;
                    float hR = (x < config_.width_segments) ? heights[z][x + 1] : pos_y;
                    float hD = (z > 0) ? heights[z - 1][x] : pos_y;
                    float hU = (z < config_.depth_segments) ? heights[z + 1][x] : pos_y;
                    
                    // Calcular vectores tangentes
                    glm::vec3 tangent_x(2.0f * x_step, hR - hL, 0.0f);
                    glm::vec3 tangent_z(0.0f, hU - hD, 2.0f * z_step);
                    
                    // Normal es el producto cruz de los tangentes
                    normal = glm::normalize(glm::cross(tangent_z, tangent_x));
                }
                
                // Agregar vértice: posición, normal, coordenadas de textura
                vertices_.insert(vertices_.end(), {
                    pos_x, pos_y, pos_z,     // posición
                    normal.x, normal.y, normal.z,  // normal  
                    u, v                      // coordenadas de textura
                });
            }
        }
        
        vertex_count_ = (config_.width_segments + 1) * (config_.depth_segments + 1);
    }

    void Terrain::generateIndices() {
        indices_.clear();
        
        // Usar GL_TRIANGLES con índices compartidos (más simple y sin artefactos)
        // Cada quad (cuadrado) se divide en 2 triángulos
        // Los vértices se reutilizan automáticamente gracias a EBO
        
        for (int z = 0; z < config_.depth_segments; ++z) {
            for (int x = 0; x < config_.width_segments; ++x) {
                // Calcular índices de los cuatro vértices del quad
                int top_left = z * (config_.width_segments + 1) + x;
                int top_right = top_left + 1;
                int bottom_left = (z + 1) * (config_.width_segments + 1) + x;
                int bottom_right = bottom_left + 1;
                
                // Primer triángulo (top_left, bottom_left, top_right)
                indices_.push_back(static_cast<unsigned int>(top_left));
                indices_.push_back(static_cast<unsigned int>(bottom_left));
                indices_.push_back(static_cast<unsigned int>(top_right));
                
                // Segundo triángulo (top_right, bottom_left, bottom_right)
                indices_.push_back(static_cast<unsigned int>(top_right));
                indices_.push_back(static_cast<unsigned int>(bottom_left));
                indices_.push_back(static_cast<unsigned int>(bottom_right));
            }
        }
        
        index_count_ = indices_.size();
        
        // Información de optimización
        int vertices_total = (config_.width_segments + 1) * (config_.depth_segments + 1);
        int triangles = config_.width_segments * config_.depth_segments * 2;
        
        std::cout << "Terrain mesh info:" << std::endl;
        std::cout << "  Vertices: " << vertices_total << std::endl;
        std::cout << "  Triangles: " << triangles << std::endl;
        std::cout << "  Indices: " << index_count_ << std::endl;
        std::cout << "  Vertex reuse: " << static_cast<float>(index_count_) / vertices_total << "x" << std::endl;
    }

    void Terrain::setupBuffers() {
        // Generar y configurar VAO
        glGenVertexArrays(1, &VAO_);
        glBindVertexArray(VAO_);
        
        // Generar y configurar VBO
        glGenBuffers(1, &VBO_);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_);
        glBufferData(GL_ARRAY_BUFFER, vertices_.size() * sizeof(float), vertices_.data(), GL_STATIC_DRAW);
        
        // Generar y configurar EBO
        glGenBuffers(1, &EBO_);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_.size() * sizeof(unsigned int), indices_.data(), GL_STATIC_DRAW);
        
        // Configurar atributos de vértice
        // Posición (location = 0)
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        
        // Normal (location = 1)
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        
        // Coordenadas de textura (location = 2)
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);
        
        glBindVertexArray(0);
    }

    void Terrain::draw() const {
        if (VAO_ == 0) return;
        
        glBindVertexArray(VAO_);
        glDrawElements(GL_TRIANGLES, index_count_, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    void Terrain::cleanup() {
        if (VAO_) {
            glDeleteVertexArrays(1, &VAO_);
            VAO_ = 0;
        }
        if (VBO_) {
            glDeleteBuffers(1, &VBO_);
            VBO_ = 0;
        }
        if (EBO_) {
            glDeleteBuffers(1, &EBO_);
            EBO_ = 0;
        }
        
        vertices_.clear();
        indices_.clear();
        vertex_count_ = 0;
        index_count_ = 0;
    }

    float Terrain::getHeightAt(float x, float z) const {
        // Si no usa Perlin Noise, devolver altura base
        if (!config_.use_perlin_noise) {
            return config_.y_position;
        }
        
        // Recrear el generador de Perlin Noise con la misma semilla
        Utils::PerlinNoise perlin(config_.noise_seed);
        
        // Calcular altura usando los mismos parámetros que en la generación
        float height = config_.y_position + perlin.getTerrainHeight(
            x, z,
            config_.noise_scale,
            config_.height_multiplier,
            config_.noise_octaves
        );
        
        return height;
    }

} // namespace Scene