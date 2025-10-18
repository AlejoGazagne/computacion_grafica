// NEW FILE: src/ui/text_renderer_atlas.cpp
#include "text_renderer_atlas.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

namespace UI
{

  // Definir información de cada dígito en un atlas simple 10x1 (cada dígito en una columna)
  // Coordenadas UV normalizadas (0.0 a 1.0)
  CharInfo TextRendererAtlas::digits_[DIGITS_COUNT] = {
      {0.0f, 0.1f, 0.0f, 1.0f, 1.0f}, // 0
      {0.1f, 0.2f, 0.0f, 1.0f, 1.0f}, // 1
      {0.2f, 0.3f, 0.0f, 1.0f, 1.0f}, // 2
      {0.3f, 0.4f, 0.0f, 1.0f, 1.0f}, // 3
      {0.4f, 0.5f, 0.0f, 1.0f, 1.0f}, // 4
      {0.5f, 0.6f, 0.0f, 1.0f, 1.0f}, // 5
      {0.6f, 0.7f, 0.0f, 1.0f, 1.0f}, // 6
      {0.7f, 0.8f, 0.0f, 1.0f, 1.0f}, // 7
      {0.8f, 0.9f, 0.0f, 1.0f, 1.0f}, // 8
      {0.9f, 1.0f, 0.0f, 1.0f, 1.0f}  // 9
  };

  TextRendererAtlas::QuadData TextRendererAtlas::getDigitQuad(
      int digit,
      float x, float y,
      float height)
  {
    if (digit < 0 || digit >= DIGITS_COUNT)
    {
      digit = 0;
    }

    const CharInfo &info = digits_[digit];
    float width = height * info.width_ratio;

    QuadData quad;

    // Posiciones en NDC (Normalized Device Coordinates)
    quad.positions[0] = glm::vec3(x, y + height, 0.0f);         // Top-left
    quad.positions[1] = glm::vec3(x, y, 0.0f);                  // Bottom-left
    quad.positions[2] = glm::vec3(x + width, y, 0.0f);          // Bottom-right
    quad.positions[3] = glm::vec3(x + width, y + height, 0.0f); // Top-right

    // Coordenadas de textura
    quad.texcoords[0] = glm::vec2(info.u_min, info.v_max); // Top-left
    quad.texcoords[1] = glm::vec2(info.u_min, info.v_min); // Bottom-left
    quad.texcoords[2] = glm::vec2(info.u_max, info.v_min); // Bottom-right
    quad.texcoords[3] = glm::vec2(info.u_max, info.v_max); // Top-right

    return quad;
  }

  std::vector<TextRendererAtlas::QuadData> TextRendererAtlas::getNumberQuads(
      int number,
      float x, float y,
      float height)
  {
    std::vector<QuadData> quads;

    std::string number_str = std::to_string(std::abs(number));

    float current_x = x - (number_str.length() * height * 0.6f); // Center

    for (char c : number_str)
    {
      int digit = c - '0';
      QuadData quad = getDigitQuad(digit, current_x, y, height);
      quads.push_back(quad);
      current_x += height * 1.2f; // Spacing between characters
    }

    return quads;
  }

  bool TextRendererAtlas::loadAtlas(const std::string &texture_path)
  {
    // En una implementación real, aquí cargaríamos la textura del atlas
    // Por ahora, asumimos que la textura está precargada en el TextureManager
    return true;
  }

}
