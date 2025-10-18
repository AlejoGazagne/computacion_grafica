// NEW FILE: src/scene/instancing.h
#ifndef INSTANCING_H
#define INSTANCING_H

#include <glm/glm.hpp>
#include <vector>

namespace Scene
{

  struct InstanceData
  {
    glm::vec3 position;
    glm::vec3 scale;
    float rotationY; // en radianes
    float billboard; // 0.0 = malla normal; 1.0 = billboard
  };

  // Configuración global de instancing
  namespace InstanceConfig
  {
    constexpr unsigned int TREE_INSTANCE_COUNT = 3000;
    constexpr float LOD_DISTANCE = 150.0f;
    constexpr float TERRAIN_SPREAD_X = 5000.0f;
    constexpr float TERRAIN_SPREAD_Z = 5000.0f;
  }

  // Utilidad para generar instancias de árboles en patrón determinista con jitter
  class InstanceGenerator
  {
  public:
    static std::vector<InstanceData> generateTreeInstances(
        unsigned int count,
        float terrain_width,
        float terrain_depth,
        bool enable_lod = true,
        float lod_distance = InstanceConfig::LOD_DISTANCE);

  private:
    static float pseudoRandom(float seed);
  };

}

#endif // INSTANCING_H
