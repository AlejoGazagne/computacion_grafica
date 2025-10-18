// NEW FILE: src/scene/instancing.cpp
#include "instancing.h"
#include <cmath>
#include <random>

namespace Scene
{

  float InstanceGenerator::pseudoRandom(float seed)
  {
    return glm::fract(glm::sin(seed) * 43758.5453f);
  }

  std::vector<InstanceData> InstanceGenerator::generateTreeInstances(
      unsigned int count,
      float terrain_width,
      float terrain_depth,
      bool enable_lod,
      float lod_distance)
  {
    std::vector<InstanceData> instances;
    instances.reserve(count);

    float grid_cols = std::sqrt(static_cast<float>(count));
    float cell_width = terrain_width / grid_cols;
    float cell_depth = terrain_depth / grid_cols;

    float start_x = -terrain_width * 0.5f;
    float start_z = -terrain_depth * 0.5f;

    unsigned int idx = 0;
    for (int gz = 0; gz < static_cast<int>(grid_cols); ++gz)
    {
      for (int gx = 0; gx < static_cast<int>(grid_cols); ++gx)
      {
        if (idx >= count)
          break;

        float base_x = start_x + gx * cell_width + cell_width * 0.5f;
        float base_z = start_z + gz * cell_depth + cell_depth * 0.5f;

        // Jitter para evitar patrón perfecto de grid
        float jitter_seed = base_x + base_z * 1000.0f;
        float jitter_x = (pseudoRandom(jitter_seed) - 0.5f) * cell_width * 0.4f;
        float jitter_z = (pseudoRandom(jitter_seed + 1.0f) - 0.5f) * cell_depth * 0.4f;

        float final_x = base_x + jitter_x;
        float final_z = base_z + jitter_z;

        // Escala variada
        float scale_seed = jitter_seed + 2.0f;
        float scale_factor = 0.8f + pseudoRandom(scale_seed) * 0.4f;

        // Rotación Y variada
        float rot_seed = jitter_seed + 3.0f;
        float rotation_y = pseudoRandom(rot_seed) * 2.0f * 3.14159265f;

        // Determinar si usar billboard según distancia a origen
        float dist_to_origin = std::sqrt(final_x * final_x + final_z * final_z);
        float billboard_flag = (enable_lod && dist_to_origin > lod_distance) ? 1.0f : 0.0f;

        InstanceData inst;
        inst.position = glm::vec3(final_x, 0.0f, final_z);
        inst.scale = glm::vec3(scale_factor);
        inst.rotationY = rotation_y;
        inst.billboard = billboard_flag;

        instances.push_back(inst);
        idx++;
      }
      if (idx >= count)
        break;
    }

    return instances;
  }

}
