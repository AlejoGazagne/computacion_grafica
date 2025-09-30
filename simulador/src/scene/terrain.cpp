#include "terrain.h"
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
        
        // Calcular el paso entre vértices
        float x_step = config_.width / static_cast<float>(config_.width_segments);
        float z_step = config_.depth / static_cast<float>(config_.depth_segments);
        
        // Calcular el paso de coordenadas de textura
        float u_step = config_.texture_repeat / static_cast<float>(config_.width_segments);
        float v_step = config_.texture_repeat / static_cast<float>(config_.depth_segments);
        
        // Calcular posición inicial (centrado en el origen)
        float start_x = -config_.width * 0.5f;
        float start_z = -config_.depth * 0.5f;
        
        // Generar vértices
        for (int z = 0; z <= config_.depth_segments; ++z) {
            for (int x = 0; x <= config_.width_segments; ++x) {
                // Posición del vértice
                float pos_x = start_x + x * x_step;
                float pos_y = config_.y_position;
                float pos_z = start_z + z * z_step;
                
                // Coordenadas de textura
                float u = x * u_step;
                float v = z * v_step;
                
                // Normal (siempre apuntando hacia arriba para un terreno plano)
                float nx = 0.0f;
                float ny = 1.0f;
                float nz = 0.0f;
                
                // Agregar vértice: posición, normal, coordenadas de textura
                vertices_.insert(vertices_.end(), {
                    pos_x, pos_y, pos_z,  // posición
                    nx, ny, nz,            // normal  
                    u, v                   // coordenadas de textura
                });
            }
        }
        
        vertex_count_ = (config_.width_segments + 1) * (config_.depth_segments + 1);
    }

    void Terrain::generateIndices() {
        indices_.clear();
        
        // Generar índices para triángulos
        for (int z = 0; z < config_.depth_segments; ++z) {
            for (int x = 0; x < config_.width_segments; ++x) {
                // Calcular índices de los cuatro vértices del quad
                int top_left = z * (config_.width_segments + 1) + x;
                int top_right = top_left + 1;
                int bottom_left = (z + 1) * (config_.width_segments + 1) + x;
                int bottom_right = bottom_left + 1;
                
                // Primer triángulo (top_left, bottom_left, top_right)
                indices_.insert(indices_.end(), {
                    static_cast<unsigned int>(top_left),
                    static_cast<unsigned int>(bottom_left),
                    static_cast<unsigned int>(top_right)
                });
                
                // Segundo triángulo (top_right, bottom_left, bottom_right)
                indices_.insert(indices_.end(), {
                    static_cast<unsigned int>(top_right),
                    static_cast<unsigned int>(bottom_left),
                    static_cast<unsigned int>(bottom_right)
                });
            }
        }
        
        index_count_ = indices_.size();
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

    // Factory implementations
    std::unique_ptr<Terrain> TerrainFactory::createSmall(const std::string& name) {
        auto terrain = std::make_unique<Terrain>(name);
        auto config = TerrainConfig::createSmall();
        
        if (!terrain->initialize(config)) {
            return nullptr;
        }
        
        return terrain;
    }

    std::unique_ptr<Terrain> TerrainFactory::createFlat(const std::string& name) {
        auto terrain = std::make_unique<Terrain>(name);
        auto config = TerrainConfig::createDefault();
        
        if (!terrain->initialize(config)) {
            return nullptr;
        }
        
        return terrain;
    }

    std::unique_ptr<Terrain> TerrainFactory::createLarge(const std::string& name) {
        auto terrain = std::make_unique<Terrain>(name);
        auto config = TerrainConfig::createLarge();
        
        if (!terrain->initialize(config)) {
            return nullptr;
        }
        
        return terrain;
    }
    
    std::unique_ptr<Terrain> TerrainFactory::createInfinite(const std::string& name) {
        auto terrain = std::make_unique<Terrain>(name);
        auto config = TerrainConfig::createInfinite();
        
        if (!terrain->initialize(config)) {
            return nullptr;
        }
        
        return terrain;
    }

    std::unique_ptr<Terrain> TerrainFactory::createCustom(const TerrainConfig& config, const std::string& name) {
        auto terrain = std::make_unique<Terrain>(name);
        
        if (!terrain->initialize(config)) {
            return nullptr;
        }
        
        return terrain;
    }

} // namespace Scene