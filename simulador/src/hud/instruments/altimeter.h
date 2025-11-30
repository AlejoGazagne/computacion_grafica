#ifndef HUD_ALTIMETER_H
#define HUD_ALTIMETER_H

#include <glm/glm.hpp>
#include "../../include/hud/instrumentbase.h"
#include "graphics/rendering/buffer_objects.h"
#include <vector>
#include <memory>

namespace hud
{

  class Altimeter : public hud::InstrumentBase
  {
  public:
    Altimeter(const glm::vec2 &pos, const glm::vec2 &size, Graphics::Shaders::Shader *shader);
    ~Altimeter();

    void initialize() override;
    void update(const hud::FlightData &data) override;
    void render() override;

  protected:
    void updateModelMatrix() override;
    void clean_instrument() override {}

  private:
    bool initializeOpenGL();
    void cleanup();

    // Resources
    std::unique_ptr<Graphics::Rendering::VertexArray> vertex_array_;
    Graphics::Rendering::VertexBuffer* vertex_buffer_;

    // Helpers for drawing
    void appendLine(std::vector<float> &v, float x1, float y1, float x2, float y2);
    void drawAltitudeTape(std::vector<float> &lines);
    void drawReadoutBox(std::vector<float> &lines);
    void drawChevron(std::vector<float> &lines);

  public:
    // Layout setters (NDC space)
    void setAnchorRightX(float x) { anchor_right_x_ = x; }
    void setTickLength(float ndc_len) { tick_len_ndc_ = ndc_len; }
    void setStepNdc(float ndc_per_step) { ndc_per_step_ = ndc_per_step; }
    void setVisibleSteps(int n) { visible_steps_ = n; }
    void setBoxSize(float w_ndc, float h_ndc)
    {
      box_w_ndc_ = w_ndc;
      box_h_ndc_ = h_ndc;
    }
    void setChevronWidth(float ndc) { chevron_width_ndc_ = ndc; }

  private:
    float altitude_ft_ = 0.0f;

    // Visual layout in NDC (configurable)
    float anchor_right_x_ = 0.85f; // Right-side tape anchor X
    float tick_len_ndc_ = 0.05f;   // Tick length in NDC
    const float kStepFt_ = 100.0f; // 100 ft per step
    float ndc_per_step_ = 0.05f;   // Vertical spacing in NDC per 100 ft
    int visible_steps_ = 12;       // Visible steps above and below

    // Readout box and chevron in NDC
    float box_w_ndc_ = 0.25f;
    float box_h_ndc_ = 0.08f;
    float chevron_width_ndc_ = 0.02f;
  };

} // namespace hud

#endif // HUD_ALTIMETER_H
