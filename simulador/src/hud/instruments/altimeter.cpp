#include "altimeter.h"
#include "../text_renderer.h"
#include "../graphics/shaders/shader_manager.h"

extern "C"
{
#include <glad/glad.h>
}

#include <cmath>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>

namespace UI
{

  Altimeter::Altimeter(const glm::vec2 &pos, const glm::vec2 &size, Graphics::Shaders::Shader *shader)
      : hud::InstrumentBase(pos, size, shader) {}

  Altimeter::~Altimeter() { cleanup(); }

  bool Altimeter::initializeOpenGL()
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

  void Altimeter::cleanup() { clean(); }

  void Altimeter::initialize()
  {
    initializeOpenGL();
  }

  void Altimeter::update(const hud::FlightData &data)
  {
    altitude_ft_ = data.altitude;
    updateModelMatrix();
  }

  void Altimeter::appendLine(std::vector<float> &v, float x1, float y1, float x2, float y2)
  {
    v.push_back(x1);
    v.push_back(y1);
    v.push_back(x2);
    v.push_back(y2);
  }

  void Altimeter::drawAltitudeTape(std::vector<float> &lines)
  {
    // Determine base step and scroll offset
    float base = std::floor(altitude_ft_ / kStepFt_) * kStepFt_;
    float frac = (altitude_ft_ - base) / kStepFt_;
    float scroll = frac * ndc_per_step_;

    float centerY = 0.0f; // centered in NDC

    for (int i = -visible_steps_; i <= visible_steps_; ++i)
    {
      float markFt = base + i * kStepFt_;
      float y = centerY + scroll - i * ndc_per_step_;
      if (y < -1.1f || y > 1.1f)
        continue; // simple cull

      // Skip if inside readout box region to keep it clean
      if (y > -box_h_ndc_ * 0.5f && y < box_h_ndc_ * 0.5f)
        continue;

      // Tick to the left from kRightX_
      appendLine(lines, anchor_right_x_ - tick_len_ndc_, y, anchor_right_x_, y);

      // Number every step (smaller digits, tighter to tick)
      if (markFt >= 0.0f)
      {
        float clearance = 0.006f;
        float digit_h = 0.020f;
        float digit_w = 0.013f;
        int digits = (int)std::to_string((int)markFt).length();
        // center so that right edge keeps 'clearance' from tick end
        float textX = anchor_right_x_ - tick_len_ndc_ - (0.6f * digits * digit_w + clearance);
        // place fully below the tick to avoid overlap
        float textY = y - digit_h - 0.004f;
        std::vector<float> num = TextRenderer::generateNumberVertices((int)markFt, textX, textY, digit_w, digit_h);
        lines.insert(lines.end(), num.begin(), num.end());
      }
    }
  }

  void Altimeter::drawChevron(std::vector<float> &lines)
  {
    float centerY = 0.0f;
    float boxLeft = anchor_right_x_ - box_w_ndc_;
    float chevronX = boxLeft - chevron_width_ndc_;
    float chevronTop = centerY - (box_h_ndc_ * 0.5f);
    float chevronBot = centerY + (box_h_ndc_ * 0.5f);

    appendLine(lines, chevronX, chevronTop, boxLeft, centerY);
    appendLine(lines, boxLeft, centerY, chevronX, chevronBot);
    appendLine(lines, chevronX, chevronTop, chevronX, chevronBot);
  }

  void Altimeter::drawReadoutBox(std::vector<float> &lines)
  {
    float centerY = 0.0f;
    float x1 = anchor_right_x_ - box_w_ndc_;
    float y1 = centerY - box_h_ndc_ * 0.5f;
    float x2 = anchor_right_x_;
    float y2 = centerY + box_h_ndc_ * 0.5f;

    // Rectangle outline
    appendLine(lines, x1, y1, x2, y1);
    appendLine(lines, x2, y1, x2, y2);
    appendLine(lines, x2, y2, x1, y2);
    appendLine(lines, x1, y2, x1, y1);

    // Current altitude number centered in box with proportional size
    int display = (int)std::round(std::max(0.0f, altitude_ft_));
    float centerX = (x1 + x2) * 0.5f;
    float digit_h = box_h_ndc_ * 0.50f;
    float digit_w = digit_h * 0.66f;
    float textY = centerY - digit_h * 0.75f;
    std::vector<float> num = TextRenderer::generateNumberVertices(display, centerX, textY, digit_w, digit_h);
    lines.insert(lines.end(), num.begin(), num.end());
  }

  void Altimeter::render()
  {
    auto *shader = shader_;
    if (!shader || !shader->isCompiled())
      return;

    GLboolean depthWasEnabled = glIsEnabled(GL_DEPTH_TEST);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    shader->use();
    // Set ortho projection over NDC space
    shader->setMat4("projection", glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f));
    shader->setVec3("color", 0.0f, 1.0f, 0.0f);
    shader->setFloat("alpha", 0.85f);

    std::vector<float> lines;

    // Tape
    drawAltitudeTape(lines);
    // Readout box
    drawReadoutBox(lines);
    // Chevron
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

  void Altimeter::updateModelMatrix() { model_matrix_ = glm::mat4(1.0f); }

} // namespace UI
