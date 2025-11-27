#include "speed_indicator.h"
#include "../text_renderer.h"
#include "../graphics/shaders/shader_manager.h"

extern "C"
{
#include <glad/glad.h>
}

#include <cmath>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>

namespace hud
{

  SpeedIndicator::SpeedIndicator(const glm::vec2 &pos, const glm::vec2 &size, Graphics::Shaders::Shader *shader)
      : hud::InstrumentBase(pos, size, shader) {}

  SpeedIndicator::~SpeedIndicator() { cleanup(); }

  bool SpeedIndicator::initializeOpenGL()
  {
    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);

    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);

    glBindVertexArray(0);
    return true;
  }

  void SpeedIndicator::cleanup() { clean(); }

  void SpeedIndicator::initialize() { initializeOpenGL(); }

  void SpeedIndicator::update(const hud::FlightData &data)
  {
    speed_kt_ = std::max(0.0f, data.speed);
    updateModelMatrix();
  }

  void SpeedIndicator::appendLine(std::vector<float> &v, float x1, float y1, float x2, float y2)
  {
    v.push_back(x1);
    v.push_back(y1);
    v.push_back(x2);
    v.push_back(y2);
  }

  void SpeedIndicator::drawSpeedTape(std::vector<float> &lines)
  {
    float base = std::floor(speed_kt_ / kStepKt_) * kStepKt_;
    float frac = (speed_kt_ - base) / kStepKt_;
    float scroll = frac * ndc_per_step_;

    float centerY = 0.0f;

    for (int i = -visible_steps_; i <= visible_steps_; ++i)
    {
      float markKt = base + i * kStepKt_;
      if (markKt < 0.0f)
        continue;
      float y = centerY + scroll - i * ndc_per_step_;
      if (y < -1.1f || y > 1.1f)
        continue;

      // Skip inside readout
      if (y > -box_h_ndc_ * 0.5f && y < box_h_ndc_ * 0.5f)
        continue;

      // Tick from left anchor to right
      appendLine(lines, anchor_left_x_, y, anchor_left_x_ + tick_len_ndc_, y);

      // Label every 20 kt (smaller digits, closer to tick)
      if (static_cast<int>(markKt) % 20 == 0)
      {
        float clearance = 0.006f;
        float digit_h = 0.020f;
        float digit_w = 0.013f;
        int digits = (int)std::to_string((int)markKt).length();
        // center so that left edge keeps 'clearance' from tick end
        float textX = anchor_left_x_ + tick_len_ndc_ + (0.6f * digits * digit_w + clearance);
        // place fully below the tick to avoid overlap
        float textY = y - digit_h - 0.004f;
        std::vector<float> num = TextRenderer::generateNumberVertices((int)markKt, textX, textY, digit_w, digit_h);
        lines.insert(lines.end(), num.begin(), num.end());
      }
    }
  }

  void SpeedIndicator::drawChevron(std::vector<float> &lines)
  {
    float centerY = 0.0f;
    float boxRight = anchor_left_x_ + box_w_ndc_;
    float chevronX = boxRight + 0.03f;
    float chevronTop = centerY - (box_h_ndc_ * 0.5f);
    float chevronBot = centerY + (box_h_ndc_ * 0.5f);

    appendLine(lines, boxRight, centerY, chevronX, chevronTop);
    appendLine(lines, chevronX, chevronTop, chevronX, chevronBot);
    appendLine(lines, chevronX, chevronBot, boxRight, centerY);
  }

  void SpeedIndicator::drawReadoutBox(std::vector<float> &lines)
  {
    float centerY = 0.0f;
    float x1 = anchor_left_x_;
    float y1 = centerY - box_h_ndc_ * 0.5f;
    float x2 = anchor_left_x_ + box_w_ndc_;
    float y2 = centerY + box_h_ndc_ * 0.5f;

    appendLine(lines, x1, y1, x2, y1);
    appendLine(lines, x2, y1, x2, y2);
    appendLine(lines, x2, y2, x1, y2);
    appendLine(lines, x1, y2, x1, y1);

    int display = (int)std::round(speed_kt_);
    float centerX = (x1 + x2) * 0.5f;
    float digit_h = box_h_ndc_ * 0.50f;
    float digit_w = digit_h * 0.66f;
    float textY = centerY - digit_h * 0.75f;
    std::vector<float> num = TextRenderer::generateNumberVertices(display, centerX, textY, digit_w, digit_h);
    lines.insert(lines.end(), num.begin(), num.end());
  }

  void SpeedIndicator::render()
  {
    auto *shader = shader_;
    if (!shader || !shader->isCompiled())
      return;

    GLboolean depthWasEnabled = glIsEnabled(GL_DEPTH_TEST);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    shader->use();
    shader->setMat4("projection", glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f));
    shader->setVec3("color", 0.0f, 1.0f, 0.0f);
    shader->setFloat("alpha", 0.85f);

    std::vector<float> lines;
    drawSpeedTape(lines);
    drawReadoutBox(lines);
    drawChevron(lines);

    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, lines.size() * sizeof(float), lines.data(), GL_DYNAMIC_DRAW);
    glDrawArrays(GL_LINES, 0, (GLsizei)(lines.size() / 2));

    glBindVertexArray(0);
    glUseProgram(0);
    if (depthWasEnabled)
      glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
  }

  void SpeedIndicator::updateModelMatrix() { model_matrix_ = glm::mat4(1.0f); }

} // namespace hud
