#pragma once

#include <memory>
#include <string>
#include <vector>
#include <glm/glm.hpp>

namespace Scene {

    struct TerrainConfig {
        float width = 200.0f;          // Mucho más grande por defecto
        float depth = 200.0f;
        float y_position = -2.0f;      // Posición Y del piso (debajo del cubo)
        int width_segments = 40;       // Más segmentos para suavidad
        int depth_segments = 40;
        std::string texture_name = "terrain";
        glm::vec3 color = glm::vec3(0.8f, 0.8f, 0.8f);
        float texture_repeat = 50.0f;  // Repetir textura muchas veces
        
        static TerrainConfig createDefault() {
            return TerrainConfig{};
        }
        
        static TerrainConfig createSmall() {
            TerrainConfig config;
            config.width = 20.0f;
            config.depth = 20.0f;
            config.width_segments = 10;
            config.depth_segments = 10;
            config.texture_repeat = 4.0f;
            return config;
        }
        
        static TerrainConfig createLarge() {
            TerrainConfig config;
            config.width = 500.0f;       // Muy grande - hasta el horizonte
            config.depth = 500.0f;
            config.width_segments = 50;  // Optimizado para distancia
            config.depth_segments = 50;
            config.texture_repeat = 100.0f;
            return config;
        }
        
        static TerrainConfig createInfinite() {
            TerrainConfig config;
            config.width = 1000.0f;      // Simula terrain "infinito"
            config.depth = 1000.0f;
            config.width_segments = 60;
            config.depth_segments = 60;
            config.texture_repeat = 200.0f;
            return config;
        }
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
        bool initialize(const TerrainConfig& config = TerrainConfig::createDefault());
        
        // Rendering
        void draw() const;
        
        // Getters
        const std::string& getName() const { return name_; }
        const TerrainConfig& getConfig() const { return config_; }
        unsigned int getVertexCount() const { return vertex_count_; }
        unsigned int getIndexCount() const { return index_count_; }
        glm::vec3 getPosition() const { return glm::vec3(0.0f, config_.y_position, 0.0f); }
        
        // Setters
        void setPosition(float y) { config_.y_position = y; }
        void setTextureRepeat(float repeat) { config_.texture_repeat = repeat; }
    };

    // Factory class for terrain creation
    class TerrainFactory {
    public:
        static std::unique_ptr<Terrain> createSmall(const std::string& name = "small_terrain");
        static std::unique_ptr<Terrain> createFlat(const std::string& name = "flat_terrain");        
        static std::unique_ptr<Terrain> createLarge(const std::string& name = "large_terrain");
        static std::unique_ptr<Terrain> createInfinite(const std::string& name = "infinite_terrain");
        static std::unique_ptr<Terrain> createCustom(const TerrainConfig& config, const std::string& name = "custom_terrain");
    };

} // namespace Scene