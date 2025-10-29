#pragma once

#include <glm/glm.hpp>
#include <unordered_map>
#include <vector>
#include <string>

namespace Scene {

// Parámetros base tomados de TerrainConfig (cada chunk tendrá este tamaño)
struct ChunkedTerrainConfig {
    float chunk_width;      // tamaño del chunk en X (mismo que TerrainConfig::width)
    float chunk_depth;      // tamaño del chunk en Z (mismo que TerrainConfig::depth)
    float y_position;       // nivel base
    int width_segments;     // segmentos por chunk (X)
    int depth_segments;     // segmentos por chunk (Z)
    float texture_repeat;   // repetición UV por chunk
    // Perlin noise
    bool use_perlin_noise;
    float noise_scale;
    float height_multiplier;
    int noise_octaves;
    unsigned int noise_seed;

    // Streaming
    int view_radius_chunks = 2; // radio de chunks alrededor de la cámara (2 -> 5x5)
};

class ChunkedTerrain {
public:
    explicit ChunkedTerrain(const std::string &name = "chunked_terrain");
    ~ChunkedTerrain();

    bool initialize(const ChunkedTerrainConfig &cfg);

    // Actualiza la malla de chunks alrededor de la cámara
    void update(const glm::vec3 &camera_pos);

    // Dibuja todos los chunks activos (asume que el shader ya tiene view/projection/model = I)
    void draw() const;

    // Altura en un punto del mundo (coincide con la función usada para generar)
    float getHeightAt(float x, float z) const;

    const ChunkedTerrainConfig &getConfig() const { return config_; }

private:
    struct ChunkKey {
        int gx;
        int gz;
        bool operator==(const ChunkKey &o) const { return gx == o.gx && gz == o.gz; }
    };

    struct ChunkKeyHasher {
        std::size_t operator()(const ChunkKey &k) const {
            return (std::hash<int>()(k.gx) * 73856093) ^ (std::hash<int>()(k.gz) * 19349663);
        }
    };

    struct Chunk {
        unsigned int VAO = 0;
        unsigned int VBO = 0;
        unsigned int EBO = 0;
        unsigned int index_count = 0;
        glm::vec2 origin; // centro del chunk en XZ (mundo)
    };

    void ensureChunk(int gx, int gz);
    void createChunk(int gx, int gz);
    void destroyChunk(Chunk &c);
    void evictFarChunks(int center_gx, int center_gz);

    // Generación de geometría (similar a Terrain::generateVertices/Indices pero con offset)
    void buildChunkMesh(float origin_x, float origin_z,
                        std::vector<float> &out_vertices,
                        std::vector<unsigned int> &out_indices);

private:
    std::string name_;
    ChunkedTerrainConfig config_{};
    std::unordered_map<ChunkKey, Chunk, ChunkKeyHasher> chunks_;
};

} // namespace Scene
