#include "chunked_terrain.h"
#include "terrain.h" // Para reutilizar tipos y coherencia
#include "../utils/perlin_noise.h"
#include <glad/glad.h>
#include <cmath>
#include <iostream>

namespace Scene {

ChunkedTerrain::ChunkedTerrain(const std::string &name) : name_(name) {}

ChunkedTerrain::~ChunkedTerrain() {
    for (auto &kv : chunks_) {
        destroyChunk(kv.second);
    }
    chunks_.clear();
}

bool ChunkedTerrain::initialize(const ChunkedTerrainConfig &cfg) {
    config_ = cfg;
    // No generamos nada aún, se hará en update() según la cámara
    std::cout << "ChunkedTerrain '" << name_ << "' initialized (chunk "
              << config_.chunk_width << "x" << config_.chunk_depth
              << ", segments " << config_.width_segments << "x" << config_.depth_segments
              << ", radius " << config_.view_radius_chunks << ")" << std::endl;
    return true;
}

void ChunkedTerrain::update(const glm::vec3 &camera_pos) {
    // Determinar en qué celda de la grilla de chunks está la cámara
    int gx = static_cast<int>(std::floor(camera_pos.x / config_.chunk_width));
    int gz = static_cast<int>(std::floor(camera_pos.z / config_.chunk_depth));

    // Asegurar todos los chunks dentro del radio
    for (int dz = -config_.view_radius_chunks; dz <= config_.view_radius_chunks; ++dz) {
        for (int dx = -config_.view_radius_chunks; dx <= config_.view_radius_chunks; ++dx) {
            ensureChunk(gx + dx, gz + dz);
        }
    }

    // Eliminar los que queden muy lejos
    evictFarChunks(gx, gz);
}

void ChunkedTerrain::draw() const {
    for (const auto &kv : chunks_) {
        const Chunk &c = kv.second;
        if (c.VAO == 0 || c.index_count == 0) continue;
        glBindVertexArray(c.VAO);
        glDrawElements(GL_TRIANGLES, c.index_count, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
}

float ChunkedTerrain::getHeightAt(float x, float z) const {
    if (!config_.use_perlin_noise) return config_.y_position;
    Utils::PerlinNoise perlin(config_.noise_seed);
    float h = config_.y_position + perlin.getTerrainHeight(
        x, z, config_.noise_scale, config_.height_multiplier, config_.noise_octaves);
    return h;
}

void ChunkedTerrain::ensureChunk(int gx, int gz) {
    ChunkKey key{gx, gz};
    if (chunks_.find(key) != chunks_.end()) return;
    createChunk(gx, gz);
}

void ChunkedTerrain::createChunk(int gx, int gz) {
    ChunkKey key{gx, gz};
    Chunk chunk;
    chunk.origin = glm::vec2((gx + 0.5f) * config_.chunk_width, (gz + 0.5f) * config_.chunk_depth);

    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    buildChunkMesh(chunk.origin.x, chunk.origin.y, vertices, indices);

    // Crear buffers
    glGenVertexArrays(1, &chunk.VAO);
    glBindVertexArray(chunk.VAO);

    glGenBuffers(1, &chunk.VBO);
    glBindBuffer(GL_ARRAY_BUFFER, chunk.VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &chunk.EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, chunk.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // Atributos (pos 0..2, normal 3..5, uv 6..7)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    chunk.index_count = static_cast<unsigned int>(indices.size());
    chunks_.emplace(key, std::move(chunk));
}

void ChunkedTerrain::destroyChunk(Chunk &c) {
    if (c.VAO) glDeleteVertexArrays(1, &c.VAO);
    if (c.VBO) glDeleteBuffers(1, &c.VBO);
    if (c.EBO) glDeleteBuffers(1, &c.EBO);
    c = Chunk{};
}

void ChunkedTerrain::evictFarChunks(int center_gx, int center_gz) {
    std::vector<ChunkKey> to_remove;
    for (const auto &kv : chunks_) {
        int dx = std::abs(kv.first.gx - center_gx);
        int dz = std::abs(kv.first.gz - center_gz);
        if (dx > config_.view_radius_chunks || dz > config_.view_radius_chunks) {
            to_remove.push_back(kv.first);
        }
    }
    for (const auto &k : to_remove) {
        auto it = chunks_.find(k);
        if (it != chunks_.end()) {
            destroyChunk(it->second);
            chunks_.erase(it);
        }
    }
}

void ChunkedTerrain::buildChunkMesh(float origin_x, float origin_z,
                                    std::vector<float> &out_vertices,
                                    std::vector<unsigned int> &out_indices) {
    out_vertices.clear();
    out_indices.clear();

    Utils::PerlinNoise perlin(config_.noise_seed);

    // Pasos
    float x_step = config_.chunk_width / static_cast<float>(config_.width_segments);
    float z_step = config_.chunk_depth / static_cast<float>(config_.depth_segments);
    float u_step = config_.texture_repeat / static_cast<float>(config_.width_segments);
    float v_step = config_.texture_repeat / static_cast<float>(config_.depth_segments);

    // Inicio (centrado en el origin)
    float start_x = origin_x - config_.chunk_width * 0.5f;
    float start_z = origin_z - config_.chunk_depth * 0.5f;

    // Almacenar alturas para normales
    std::vector<std::vector<float>> heights(config_.depth_segments + 1,
                                            std::vector<float>(config_.width_segments + 1));

    for (int z = 0; z <= config_.depth_segments; ++z) {
        for (int x = 0; x <= config_.width_segments; ++x) {
            float pos_x = start_x + x * x_step;
            float pos_z = start_z + z * z_step;
            float height = config_.y_position;
            if (config_.use_perlin_noise) {
                height = config_.y_position + perlin.getTerrainHeight(
                    pos_x, pos_z,
                    config_.noise_scale,
                    config_.height_multiplier,
                    config_.noise_octaves);
            }
            heights[z][x] = height;
        }
    }

    // Vértices
    for (int z = 0; z <= config_.depth_segments; ++z) {
        for (int x = 0; x <= config_.width_segments; ++x) {
            float pos_x = start_x + x * x_step;
            float pos_y = heights[z][x];
            float pos_z = start_z + z * z_step;

            float u = x * u_step;
            float v = z * v_step;

            // Normal
            glm::vec3 normal(0.0f, 1.0f, 0.0f);
            if (config_.use_perlin_noise) {
                float hL = (x > 0) ? heights[z][x - 1] : pos_y;
                float hR = (x < config_.width_segments) ? heights[z][x + 1] : pos_y;
                float hD = (z > 0) ? heights[z - 1][x] : pos_y;
                float hU = (z < config_.depth_segments) ? heights[z + 1][x] : pos_y;
                glm::vec3 tangent_x(2.0f * x_step, hR - hL, 0.0f);
                glm::vec3 tangent_z(0.0f, hU - hD, 2.0f * z_step);
                normal = glm::normalize(glm::cross(tangent_z, tangent_x));
            }

            out_vertices.insert(out_vertices.end(), {
                pos_x, pos_y, pos_z,
                normal.x, normal.y, normal.z,
                u, v
            });
        }
    }

    // Indices (triángulos)
    for (int z = 0; z < config_.depth_segments; ++z) {
        for (int x = 0; x < config_.width_segments; ++x) {
            int top_left = z * (config_.width_segments + 1) + x;
            int top_right = top_left + 1;
            int bottom_left = (z + 1) * (config_.width_segments + 1) + x;
            int bottom_right = bottom_left + 1;

            out_indices.push_back(static_cast<unsigned int>(top_left));
            out_indices.push_back(static_cast<unsigned int>(bottom_left));
            out_indices.push_back(static_cast<unsigned int>(top_right));

            out_indices.push_back(static_cast<unsigned int>(top_right));
            out_indices.push_back(static_cast<unsigned int>(bottom_left));
            out_indices.push_back(static_cast<unsigned int>(bottom_right));
        }
    }
}

} // namespace Scene
