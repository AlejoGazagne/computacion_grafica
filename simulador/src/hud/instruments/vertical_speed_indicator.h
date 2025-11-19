#ifndef UI_VERTICAL_SPEED_INDICATOR_H
#define UI_VERTICAL_SPEED_INDICATOR_H

#include <glm/glm.hpp>
#include "../../include/hud/instrumentbase.h"
#include <vector>

namespace UI
{

  class VerticalSpeedIndicator : public hud::InstrumentBase
  {
  public:
    VerticalSpeedIndicator(const glm::vec2 &pos, const glm::vec2 &size, Graphics::Shaders::Shader *shader);
    ~VerticalSpeedIndicator();

    void initialize() override;
    void update(const hud::FlightData &data) override;
    void render() override;

  protected:
    void updateModelMatrix() override;
    void clean_instrument() override {}

  private:
    bool initializeOpenGL();
    void cleanup();

    void appendLine(std::vector<float> &v, float x1, float y1, float x2, float y2);
    void drawScale(std::vector<float> &lines);
    void drawIndicator(std::vector<float> &lines);
    void drawReadout(std::vector<float> &lines);

  private:
    float vsi_fpm_ = 0.0f; // feet per minute

    // Layout near right side but left of altimeter (configurable)
    float line_x_ndc_ = 0.45f;         // Vertical scale line X
    float scale_h_ndc_ = 0.9f;         // Total NDC height span
    float tick_len_ndc_ = 0.03f;       // Minor tick length
    float major_tick_len_ndc_ = 0.05f; // Major tick length

    // Ranges and display
    const float kMaxFpm_ = 6000.0f;
    const float kMinFpm_ = -6000.0f;
    const float kMarkInterval_ = 1000.0f; // ft/min
    const float kDisplayScale_ = 100.0f;  // show divided by 100

    // Readout box
    float box_w_ndc_ = 0.12f;
    float box_h_ndc_ = 0.05f;

  public:
    // Layout setters
    void setLineX(float x) { line_x_ndc_ = x; }
    void setScaleHeight(float h) { scale_h_ndc_ = h; }
    void setTickLengths(float minorNdc, float majorNdc)
    {
      tick_len_ndc_ = minorNdc;
      major_tick_len_ndc_ = majorNdc;
    }
    void setBoxSize(float w, float h)
    {
      box_w_ndc_ = w;
      box_h_ndc_ = h;
    }
  };

} // namespace UI

#endif // UI_VERTICAL_SPEED_INDICATOR_H
