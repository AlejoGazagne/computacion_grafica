#ifndef UI_SPEED_INDICATOR_H
#define UI_SPEED_INDICATOR_H

#include <glm/glm.hpp>
#include "../../include/hud/instrumentbase.h"
#include <vector>

namespace UI
{

  class SpeedIndicator : public hud::InstrumentBase
  {
  public:
    SpeedIndicator(const glm::vec2 &pos, const glm::vec2 &size, Graphics::Shaders::Shader *shader);
    ~SpeedIndicator();

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
    void drawSpeedTape(std::vector<float> &lines);
    void drawReadoutBox(std::vector<float> &lines);
    void drawChevron(std::vector<float> &lines);

  public:
    // Layout setters (NDC space)
    void setAnchorLeftX(float x) { anchor_left_x_ = x; }
    void setTickLength(float ndc_len) { tick_len_ndc_ = ndc_len; }
    void setStepNdc(float ndc_per_step) { ndc_per_step_ = ndc_per_step; }
    void setVisibleSteps(int n) { visible_steps_ = n; }
    void setBoxSize(float w_ndc, float h_ndc)
    {
      box_w_ndc_ = w_ndc;
      box_h_ndc_ = h_ndc;
    }

  private:
    float speed_kt_ = 0.0f;

    // Layout (left side), configurable
    float anchor_left_x_ = -0.85f; // Left tape anchor
    float tick_len_ndc_ = 0.05f;
    const float kStepKt_ = 10.0f;
    float ndc_per_step_ = 0.05f; // spacing per 10 kt
    int visible_steps_ = 12;

    float box_w_ndc_ = 0.22f;
    float box_h_ndc_ = 0.08f;
  };

} // namespace UI

#endif // UI_SPEED_INDICATOR_H
