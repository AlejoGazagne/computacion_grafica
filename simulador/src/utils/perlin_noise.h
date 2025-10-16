#pragma once

#include <vector>
#include <cmath>
#include <random>
#include <algorithm>

namespace Utils {

/**
 * @brief Implementación simple de Perlin Noise para generación procedural de terrenos
 * Basado en el algoritmo clásico de Ken Perlin
 */
class PerlinNoise {
private:
    std::vector<int> permutation_;
    
    // Función de interpolación suave (smoothstep)
    static float fade(float t) {
        return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
    }
    
    // Interpolación lineal
    static float lerp(float t, float a, float b) {
        return a + t * (b - a);
    }
    
    // Función de gradiente
    static float grad(int hash, float x, float y, float z) {
        int h = hash & 15;
        float u = h < 8 ? x : y;
        float v = h < 4 ? y : h == 12 || h == 14 ? x : z;
        return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
    }

public:
    /**
     * @brief Constructor que inicializa la tabla de permutación
     * @param seed Semilla para generación aleatoria (opcional)
     */
    PerlinNoise(unsigned int seed = 237) {
        permutation_.resize(512);
        
        // Llenar con valores 0-255
        std::vector<int> p(256);
        for (int i = 0; i < 256; i++) {
            p[i] = i;
        }
        
        // Mezclar usando seed
        std::default_random_engine engine(seed);
        std::shuffle(p.begin(), p.end(), engine);
        
        // Duplicar para evitar desbordamiento
        for (int i = 0; i < 512; i++) {
            permutation_[i] = p[i & 255];
        }
    }
    
    /**
     * @brief Calcula el valor de Perlin Noise en un punto 3D
     * @param x Coordenada X
     * @param y Coordenada Y
     * @param z Coordenada Z
     * @return Valor de ruido en el rango [-1, 1]
     */
    float noise(float x, float y, float z) const {
        // Encontrar la celda unitaria que contiene el punto
        int X = static_cast<int>(std::floor(x)) & 255;
        int Y = static_cast<int>(std::floor(y)) & 255;
        int Z = static_cast<int>(std::floor(z)) & 255;
        
        // Encontrar posición relativa del punto en la celda
        x -= std::floor(x);
        y -= std::floor(y);
        z -= std::floor(z);
        
        // Calcular curvas de fade
        float u = fade(x);
        float v = fade(y);
        float w = fade(z);
        
        // Hashes de las 8 esquinas del cubo
        int A = permutation_[X] + Y;
        int AA = permutation_[A] + Z;
        int AB = permutation_[A + 1] + Z;
        int B = permutation_[X + 1] + Y;
        int BA = permutation_[B] + Z;
        int BB = permutation_[B + 1] + Z;
        
        // Interpolar entre los 8 gradientes
        return lerp(w,
            lerp(v,
                lerp(u, grad(permutation_[AA], x, y, z),
                        grad(permutation_[BA], x - 1, y, z)),
                lerp(u, grad(permutation_[AB], x, y - 1, z),
                        grad(permutation_[BB], x - 1, y - 1, z))),
            lerp(v,
                lerp(u, grad(permutation_[AA + 1], x, y, z - 1),
                        grad(permutation_[BA + 1], x - 1, y, z - 1)),
                lerp(u, grad(permutation_[AB + 1], x, y - 1, z - 1),
                        grad(permutation_[BB + 1], x - 1, y - 1, z - 1))));
    }
    
    /**
     * @brief Calcula el valor de Perlin Noise en 2D (z=0)
     * @param x Coordenada X
     * @param y Coordenada Y
     * @return Valor de ruido en el rango [-1, 1]
     */
    float noise2D(float x, float y) const {
        return noise(x, y, 0.0f);
    }
    
    /**
     * @brief Genera ruido fractal (múltiples octavas) para más detalle
     * @param x Coordenada X
     * @param y Coordenada Y
     * @param octaves Número de octavas (niveles de detalle)
     * @param persistence Factor de reducción de amplitud por octava (0.0-1.0)
     * @return Valor de ruido fractal en el rango aproximado [-1, 1]
     */
    float fractalNoise2D(float x, float y, int octaves = 4, float persistence = 0.5f) const {
        float total = 0.0f;
        float frequency = 1.0f;
        float amplitude = 1.0f;
        float maxValue = 0.0f;  // Para normalizar
        
        for (int i = 0; i < octaves; i++) {
            total += noise2D(x * frequency, y * frequency) * amplitude;
            
            maxValue += amplitude;
            amplitude *= persistence;
            frequency *= 2.0f;
        }
        
        return total / maxValue;  // Normalizar al rango [-1, 1]
    }
    
    /**
     * @brief Genera altura de terreno usando Perlin Noise
     * @param x Coordenada X del terreno
     * @param z Coordenada Z del terreno
     * @param scale Escala del ruido (valores más pequeños = montañas más grandes)
     * @param height_multiplier Multiplicador de altura
     * @param octaves Número de octavas para detalle
     * @return Altura del terreno en ese punto
     */
    float getTerrainHeight(float x, float z, float scale = 0.01f, float height_multiplier = 50.0f, int octaves = 4) const {
        float noise_value = fractalNoise2D(x * scale, z * scale, octaves, 0.5f);
        // Convertir de [-1, 1] a [0, height_multiplier]
        return (noise_value + 1.0f) * 0.5f * height_multiplier;
    }
};

} // namespace Utils
