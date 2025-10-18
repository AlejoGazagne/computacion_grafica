// NEW FILE: src/ui/text_renderer_atlas.h
#ifndef TEXT_RENDERER_ATLAS_H
#define TEXT_RENDERER_ATLAS_H

#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <memory>

namespace UI
{

  // Estructura para información de un carácter en el atlas
  struct CharInfo
  {
    float u_min, u_max;
    float v_min, v_max;
    float width_ratio; // Ancho relativo al alto
  };

  class TextRendererAtlas
  {
  private:
    static constexpr int DIGITS_COUNT = 10;
    static CharInfo digits_[DIGITS_COUNT];

  public:
    // Generar quad datos (posición + UV) para renderizar un dígito
    struct QuadData
    {
      glm::vec3 positions[4];
      glm::vec2 texcoords[4];
    };

    static QuadData getDigitQuad(int digit, float x, float y, float height);
    static std::vector<QuadData> getNumberQuads(
        int number,
        float x, float y,
        float height = 0.1f);

    // Cargar atlas de texturas (si fuera necesario dinámicamente)
    static bool loadAtlas(const std::string &texture_path);
  };

}

#endif // TEXT_RENDERER_ATLAS_H
