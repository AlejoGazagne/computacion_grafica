// NEW FILE: src/utils/obj_loader.cpp
#include "obj_loader.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <unordered_map>

namespace Utils
{

  std::vector<std::string> OBJLoader::split(const std::string &str, char delimiter)
  {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;
    while (std::getline(ss, token, delimiter))
    {
      if (!token.empty())
      {
        tokens.push_back(token);
      }
    }
    return tokens;
  }

  glm::vec3 OBJLoader::parseVec3(const std::string &line)
  {
    auto parts = split(line, ' ');
    float x = std::stof(parts[1]);
    float y = std::stof(parts[2]);
    float z = std::stof(parts[3]);
    return glm::vec3(x, y, z);
  }

  glm::vec2 OBJLoader::parseVec2(const std::string &line)
  {
    auto parts = split(line, ' ');
    float x = std::stof(parts[1]);
    float y = std::stof(parts[2]);
    return glm::vec2(x, y);
  }

  OBJLoader::OBJData OBJLoader::loadOBJ(const std::string &filepath)
  {
    OBJData data;
    std::ifstream file(filepath);

    if (!file.is_open())
    {
      std::cerr << "ERROR: Cannot open OBJ file: " << filepath << std::endl;
      return data;
    }

    std::vector<glm::vec3> positions;
    std::vector<glm::vec2> texcoords;
    std::vector<glm::vec3> normals;
    std::unordered_map<std::string, unsigned int> vertex_map;

    std::string line;
    while (std::getline(file, line))
    {
      if (line.empty() || line[0] == '#')
        continue;

      std::stringstream ss(line);
      std::string type;
      ss >> type;

      if (type == "v")
      {
        data.vertices.push_back(Graphics::Rendering::Vertex(parseVec3(line)));
        positions.push_back(parseVec3(line));
      }
      else if (type == "vt")
      {
        texcoords.push_back(parseVec2(line));
      }
      else if (type == "vn")
      {
        normals.push_back(parseVec3(line));
      }
      else if (type == "f")
      {
        auto parts = split(line.substr(2), ' ');
        for (size_t i = 0; i < parts.size(); ++i)
        {
          auto indices = split(parts[i], '/');
          unsigned int v_idx = std::stoul(indices[0]) - 1;
          unsigned int vt_idx = (indices.size() > 1 && !indices[1].empty())
                                    ? std::stoul(indices[1]) - 1
                                    : 0;
          unsigned int vn_idx = (indices.size() > 2 && !indices[2].empty())
                                    ? std::stoul(indices[2]) - 1
                                    : 0;

          if (v_idx < positions.size())
          {
            Graphics::Rendering::Vertex vert(positions[v_idx]);
            if (vt_idx < texcoords.size())
            {
              vert.texture_coords = texcoords[vt_idx];
            }
            if (vn_idx < normals.size())
            {
              vert.normal = normals[vn_idx];
            }

            // Búsqueda o inserción de vértice
            std::string key = std::to_string(v_idx) + "/" + std::to_string(vt_idx) + "/" + std::to_string(vn_idx);
            if (vertex_map.find(key) != vertex_map.end())
            {
              data.indices.push_back(vertex_map[key]);
            }
            else
            {
              unsigned int new_idx = static_cast<unsigned int>(data.vertices.size());
              data.vertices.push_back(vert);
              data.indices.push_back(new_idx);
              vertex_map[key] = new_idx;
            }
          }

          // Triangulate si es polígono > 3 vértices
          if (i >= 2)
          {
            auto first_key = split(parts[0], '/');
            unsigned int first_idx = std::stoul(first_key[0]) - 1;
            std::string first_str = std::to_string(first_idx) + "/0/0";
            if (vertex_map.find(first_str) != vertex_map.end())
            {
              data.indices.push_back(vertex_map[first_str]);
            }
          }
        }
      }
    }

    file.close();
    std::cout << "Loaded OBJ: " << filepath << " (" << data.vertices.size()
              << " vertices, " << data.indices.size() << " indices)" << std::endl;
    return data;
  }

}
