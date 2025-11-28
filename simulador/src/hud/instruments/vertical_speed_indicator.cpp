#include "vertical_speed_indicator.h"
#include "../text_renderer.h"
#include "../graphics/shaders/shader_manager.h"

extern "C"
{
#include <glad/glad.h>
}

#include <algorithm>
#include <cmath>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>

namespace hud
{

  VerticalSpeedIndicator::VerticalSpeedIndicator(const glm::vec2 &pos, const glm::vec2 &size, Graphics::Shaders::Shader *shader)
      : hud::InstrumentBase(pos, size, shader) {}

  VerticalSpeedIndicator::~VerticalSpeedIndicator() { cleanup(); }

  bool VerticalSpeedIndicator::initializeOpenGL()
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

  void VerticalSpeedIndicator::cleanup() { clean(); }

  void VerticalSpeedIndicator::initialize() { initializeOpenGL(); }

  void VerticalSpeedIndicator::update(const hud::FlightData &data)
  {
    vsi_fpm_ = std::isfinite(data.vertical_speed) ? data.vertical_speed : 0.0f;
    updateModelMatrix();
  }

  void VerticalSpeedIndicator::appendLine(std::vector<float> &v, float x1, float y1, float x2, float y2)
  {
    v.push_back(x1);
    v.push_back(y1);
    v.push_back(x2);
    v.push_back(y2);
  }

  void VerticalSpeedIndicator::drawScale(std::vector<float> &lines)
  {
    float top = +scale_h_ndc_ * 0.5f;
    float bot = -scale_h_ndc_ * 0.5f;
    // Backbone
    appendLine(lines, line_x_ndc_, bot, line_x_ndc_, top);

    int numMarks = (int)((kMaxFpm_ - kMinFpm_) / kMarkInterval_) + 1;
    float centerY = 0.0f;

    for (int i = 0; i < numMarks; ++i)
    {
      float vsi = kMinFpm_ + i * kMarkInterval_;
      float norm = vsi / kMaxFpm_; // -1..+1
      float y = centerY - norm * (scale_h_ndc_ * 0.5f);

      bool major = std::fmod(std::fabs(vsi), 2000.0f) < 0.1f; // every 2000
      float len = major ? major_tick_len_ndc_ : tick_len_ndc_;
      appendLine(lines, line_x_ndc_ - len, y, line_x_ndc_, y);

      if (major)
      {
        int display = (int)std::round(vsi / kDisplayScale_);
        std::string label;
        if (display == 0)
          label = "0";
        else
          label = (display > 0 ? "+" : "") + std::to_string(display);
        // place text left of ticks
        // place left of tick with clearance proportional to digits
        float digit_h = 0.016f;
        float digit_w = 0.010f;
        int digits = (int)std::to_string(std::abs(display)).length();
        float clearance = 0.006f;
        float textX = (line_x_ndc_ - len) - (0.6f * digits * digit_w + clearance);
        // place fully below tick to avoid overlap
        std::vector<float> num = TextRenderer::generateNumberVertices(std::abs(display), textX, y - digit_h - 0.004f, digit_w, digit_h);
        // Add sign as small line before number if negative
        if (display < 0)
        {
          appendLine(lines, textX - 0.02f, y - 0.004f, textX - 0.006f, y - 0.004f);
        }
        else if (display > 0)
        {
          appendLine(lines, textX - 0.016f, y - 0.004f, textX - 0.016f, y + 0.012f);
        }
        lines.insert(lines.end(), num.begin(), num.end());
      }
    }

    // Zero reference line
    appendLine(lines, line_x_ndc_ - (major_tick_len_ndc_ + 0.08f), 0.0f, line_x_ndc_ + 0.03f, 0.0f);
  }

  void VerticalSpeedIndicator::drawIndicator(std::vector<float> &lines)
  {
    float clamped = std::clamp(vsi_fpm_, kMinFpm_, kMaxFpm_);
    float norm = clamped / kMaxFpm_;
    float y = 0.0f - norm * (scale_h_ndc_ * 0.5f);

    // Triangle outline pointing to the right
    float left = line_x_ndc_ + 0.02f;
    float right = left + 0.06f;
    float top = y - 0.02f;
    float bot = y + 0.02f;

    appendLine(lines, left, top, right, y);
    appendLine(lines, right, y, left, bot);
    appendLine(lines, left, bot, left, top);
  }

  void VerticalSpeedIndicator::drawReadout(std::vector<float> &lines)
  {
    // Digital readout aligned to zero, left of scale
    float centerY = 0.0f;
    float boxRight = line_x_ndc_ - 0.06f;
    float x1 = boxRight - box_w_ndc_;
    float y1 = centerY - box_h_ndc_ * 0.5f;
    float x2 = boxRight;
    float y2 = centerY + box_h_ndc_ * 0.5f;

    appendLine(lines, x1, y1, x2, y1);
    appendLine(lines, x2, y1, x2, y2);
    appendLine(lines, x2, y2, x1, y2);
    appendLine(lines, x1, y2, x1, y1);

    int display = (int)std::round(vsi_fpm_ / kDisplayScale_);
    // center digits proportional to box size
    float centerX = (x1 + x2) * 0.5f;
    float digit_h = box_h_ndc_ * 0.50f;
    float digit_w = digit_h * 0.66f;
    float textY = centerY - digit_h * 0.75f;

    // Draw sign as a short line near left side of digits
    if (display < 0)
    {
      appendLine(lines, centerX - digit_w * 2.4f, textY + digit_h * 0.5f, centerX - digit_w * 1.6f, textY + digit_h * 0.5f);
    }
    else if (display > 0)
    {
      appendLine(lines, centerX - digit_w * 2.0f, textY + digit_h * 0.1f, centerX - digit_w * 2.0f, textY + digit_h * 0.9f);
    }

    std::vector<float> num = TextRenderer::generateNumberVertices(std::abs(display), centerX, textY, digit_w, digit_h);
    lines.insert(lines.end(), num.begin(), num.end());
  }

  void VerticalSpeedIndicator::render()
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
    drawScale(lines);
    drawIndicator(lines);
    drawReadout(lines);

    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    if (!lines.empty())
    {
      glBufferData(GL_ARRAY_BUFFER, lines.size() * sizeof(float), lines.data(), GL_DYNAMIC_DRAW);
      glDrawArrays(GL_LINES, 0, (GLsizei)(lines.size() / 2));
    }

    glBindVertexArray(0);
    glUseProgram(0);
    if (depthWasEnabled)
      glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
  }

  void VerticalSpeedIndicator::updateModelMatrix() { model_matrix_ = glm::mat4(1.0f); }

} // namespace hud
