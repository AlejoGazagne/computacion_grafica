#pragma once

#include <memory>
#include <string>
#include <vector>
#include <glm/glm.hpp>

namespace Scene {

    struct TerrainConfig {
        float width = 50000.0f;              // Terreno ultra masivo
        float depth = 50000.0f;
        float y_position = -2.0f;            // Posición Y del piso
        int width_segments = 50;            // Segmentos para detalle
        int depth_segments = 50;
        std::string texture_name = "terrain";
        glm::vec3 color = glm::vec3(0.8f, 0.8f, 0.8f);
        float texture_repeat = 100.0f;       // Repeticiones de textura
        
        // Configuración de Perlin Noise para relieve
        bool use_perlin_noise = true;
        float noise_scale = 0.0003f;         // Escala del ruido
        float height_multiplier = 800.0f;    // Altura de las montañas
        int noise_octaves = 7;               // Niveles de detalle
        unsigned int noise_seed = 237;       // Semilla
    };

    class Terrain {
    private:
        // OpenGL objects
        unsigned int VAO_, VBO_, EBO_;
        
        // Terrain properties
        TerrainConfig config_;
        std::string name_;
        
        // Mesh data
        std::vector<float> vertices_;
        std::vector<unsigned int> indices_;
        unsigned int vertex_count_;
        unsigned int index_count_;
        
        // Generation methods
        void generateVertices();
        void generateIndices();
        void setupBuffers();
        void cleanup();

    public:
        Terrain(const std::string& name = "terrain");
        ~Terrain();

        // No copiable
        Terrain(const Terrain&) = delete;
        Terrain& operator=(const Terrain&) = delete;

        // Movible
        Terrain(Terrain&& other) noexcept;
        Terrain& operator=(Terrain&& other) noexcept;

        // Initialization
        bool initialize(const TerrainConfig& config = TerrainConfig{});
        
        // Rendering
        void draw() const;
        
        // Getters
        const std::string& getName() const { return name_; }
        const TerrainConfig& getConfig() const { return config_; }
        unsigned int getVertexCount() const { return vertex_count_; }
        unsigned int getIndexCount() const { return index_count_; }
        glm::vec3 getPosition() const { return glm::vec3(0.0f, config_.y_position, 0.0f); }
        
        // Obtener altura del terreno en una posición (x, z)
        float getHeightAt(float x, float z) const;
        
        // Setters
        void setPosition(float y) { config_.y_position = y; }
        void setTextureRepeat(float repeat) { config_.texture_repeat = repeat; }
    };

} // namespace Scene

/*
FastNoiseLite - Noise generation library, usar perlin noise para generar alturas del terreno
*/