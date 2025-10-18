// NEW FILE: src/utils/obj_loader.h
#ifndef OBJ_LOADER_H
#define OBJ_LOADER_H

#include "graphics/rendering/buffer_objects.h"
#include "scene/mesh.h"
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <memory>

namespace Utils
{

  class OBJLoader
  {
  public:
    struct OBJData
    {
      std::vector<Graphics::Rendering::Vertex> vertices;
      std::vector<unsigned int> indices;
    };

    static OBJData loadOBJ(const std::string &filepath);

  private:
    static glm::vec3 parseVec3(const std::string &line);
    static glm::vec2 parseVec2(const std::string &line);
    static std::vector<std::string> split(const std::string &str, char delimiter);
  };

}

#endif // OBJ_LOADER_H
