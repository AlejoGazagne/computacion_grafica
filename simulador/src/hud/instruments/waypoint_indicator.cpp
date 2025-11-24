#include "waypoint_indicator.h"
#include "../text_renderer.h"
#include "../graphics/shaders/shader_manager.h"

extern "C"
{
#include <glad/glad.h>
}

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>
#include <vector>

namespace hud
{

  WaypointIndicator::WaypointIndicator(const glm::vec2 &pos, const glm::vec2 &size, Graphics::Shaders::Shader *shader)
      : InstrumentBase(pos, size, shader)
  {
  }

  void WaypointIndicator::initialize()
  {
    // Initialize VAO/VBO for dynamic line rendering
    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);

    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);

    // Allocate space for dynamic vertices
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 2 * 500, nullptr, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
  }

  void WaypointIndicator::update(const FlightData &data)
  {
    navData_.hasWaypoint = data.hasActiveWaypoint;
    if (!navData_.hasWaypoint)
    {
      return;
    }

    navData_.heading = data.heading;

    // Calculate relative angle to waypoint
    float bearing = data.waypointBearing;
    navData_.relativeAngle = normalizeAngle(bearing - navData_.heading);

    // Calculate altitude difference
    navData_.altitudeDifference = data.targetWaypoint.y - data.position.y;
  }

  void WaypointIndicator::render()
  {
    if (!navData_.hasWaypoint)
    {
      return; // Don't render if no waypoint
    }

    shader_->use();
    glBindVertexArray(vao_);

    renderCompassRose();
    renderVerticalIndicator();

    glBindVertexArray(0);
  }

  void WaypointIndicator::renderCompassRose()
  {
    glm::vec2 center = getCompassCenter();

    // Draw outer circle
    drawCircle(center, ROSE_RADIUS, glm::vec4(1.0f, 0.2f, 0.7f, 0.6f));

    // Draw compass ticks and numbers
    renderCompassTicks();

    // Draw waypoint pointer
    renderWaypointPointer();
  }

  void WaypointIndicator::renderCompassTicks()
  {
    glm::vec2 center = getCompassCenter();
    glm::vec4 tickColor(1.0f, 0.2f, 0.7f, 0.8f);

    std::vector<float> vertices;

    for (int i = 0; i < 72; ++i)
    {
      float angleDeg = i * 5.0f;
      float angleRad = glm::radians(angleDeg - 90.0f); // Start from top (N)

      bool isMajor = (i % 2 == 0);
      bool isNumeric = (i % 6 == 0);

      float tickLength = isMajor ? MAJOR_TICK : MINOR_TICK;
      float innerRadius = ROSE_RADIUS - tickLength;

      glm::vec2 start = center + glm::vec2(std::cos(angleRad), std::sin(angleRad)) * innerRadius;
      glm::vec2 end = center + glm::vec2(std::cos(angleRad), std::sin(angleRad)) * ROSE_RADIUS;

      // Add line segment
      vertices.push_back(start.x);
      vertices.push_back(start.y);
      vertices.push_back(end.x);
      vertices.push_back(end.y);
    }

    // Draw all tick marks
    if (!vertices.empty())
    {
      glBindBuffer(GL_ARRAY_BUFFER, vbo_);
      glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(float), vertices.data());

      shader_->setVec4("color", tickColor);

      glLineWidth(2.0f);
      glDrawArrays(GL_LINES, 0, vertices.size() / 2);
    }

    // Draw heading numbers every 30 degrees
    for (int i = 0; i < 12; ++i)
    {
      float angleDeg = i * 30.0f;
      float angleRad = glm::radians(angleDeg - 90.0f);
      int heading = static_cast<int>(angleDeg) % 360;

      float textRadius = ROSE_RADIUS + 0.06f;
      glm::vec2 textPos = center + glm::vec2(std::cos(angleRad), std::sin(angleRad)) * textRadius;

      // Draw number using TextRenderer (smaller)
      std::vector<float> numberVerts = UI::TextRenderer::generateNumberVertices(
          heading, textPos.x - 0.015f, textPos.y - 0.010f, 0.004f, 0.008f);

      if (!numberVerts.empty())
      {
        glBindBuffer(GL_ARRAY_BUFFER, vbo_);
        glBufferSubData(GL_ARRAY_BUFFER, 0, numberVerts.size() * sizeof(float), numberVerts.data());

        shader_->setVec4("color", glm::vec4(1.0f, 0.2f, 0.7f, 0.9f));

        glLineWidth(2.0f);
        glDrawArrays(GL_LINES, 0, numberVerts.size() / 2);
      }
    }
  }

  void WaypointIndicator::renderWaypointPointer()
  {
    glm::vec2 center = getCompassCenter();
    glm::vec4 pointerColor(1.0f, 0.2f, 0.7f, 1.0f);

    float pointerRad = glm::radians(navData_.relativeAngle - 90.0f);
    float pointerLength = ROSE_RADIUS * 0.75f;
    glm::vec2 tip = center + glm::vec2(std::cos(pointerRad), std::sin(pointerRad)) * pointerLength;

    std::vector<float> vertices;

    // Main arrow line
    vertices.push_back(center.x);
    vertices.push_back(center.y);
    vertices.push_back(tip.x);
    vertices.push_back(tip.y);

    // Arrow head (smaller)
    float headSize = 0.025f;
    float leftAng = pointerRad + glm::radians(150.0f);
    float rightAng = pointerRad - glm::radians(150.0f);

    glm::vec2 left = tip + glm::vec2(std::cos(leftAng), std::sin(leftAng)) * headSize;
    glm::vec2 right = tip + glm::vec2(std::cos(rightAng), std::sin(rightAng)) * headSize;

    vertices.push_back(tip.x);
    vertices.push_back(tip.y);
    vertices.push_back(left.x);
    vertices.push_back(left.y);

    vertices.push_back(tip.x);
    vertices.push_back(tip.y);
    vertices.push_back(right.x);
    vertices.push_back(right.y);

    // Draw arrow
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(float), vertices.data());

    shader_->setVec4("color", pointerColor);

    glLineWidth(3.0f);
    glDrawArrays(GL_LINES, 0, vertices.size() / 2);
  }

  void WaypointIndicator::renderVerticalIndicator()
  {
    glm::vec2 center = getCompassCenter();
    float indicatorX = center.x - ROSE_RADIUS - VERT_OFFSET;
    float indicatorY = center.y;

    std::vector<float> vertices;

    // Vertical line
    glm::vec2 lineTop(indicatorX, indicatorY - VERT_HEIGHT * 0.5f);
    glm::vec2 lineBottom(indicatorX, indicatorY + VERT_HEIGHT * 0.5f);

    vertices.push_back(lineTop.x);
    vertices.push_back(lineTop.y);
    vertices.push_back(lineBottom.x);
    vertices.push_back(lineBottom.y);

    // Center mark
    float markSize = 0.015f;
    vertices.push_back(indicatorX - markSize);
    vertices.push_back(indicatorY);
    vertices.push_back(indicatorX + markSize);
    vertices.push_back(indicatorY);

    // Draw vertical indicator lines
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(float), vertices.data());

    shader_->setVec4("color", glm::vec4(1.0f, 0.2f, 0.7f, 0.6f));

    glLineWidth(2.0f);
    glDrawArrays(GL_LINES, 0, vertices.size() / 2);

    // Calculate arrow position
    float clampedDiff = glm::clamp(navData_.altitudeDifference, -MAX_ALT_DIFF, MAX_ALT_DIFF);
    float normalizedPos = clampedDiff / MAX_ALT_DIFF;
    float arrowY = indicatorY - normalizedPos * (VERT_HEIGHT * 0.4f);

    // Determine arrow color
    glm::vec4 arrowColor;
    if (std::abs(navData_.altitudeDifference) < 50.0f)
    {
      arrowColor = glm::vec4(0.2f, 1.0f, 0.2f, 1.0f); // Green for level
    }
    else if (navData_.altitudeDifference > 0.0f)
    {
      arrowColor = glm::vec4(1.0f, 0.9f, 0.1f, 1.0f); // Yellow for up
    }
    else
    {
      arrowColor = glm::vec4(0.3f, 0.7f, 1.0f, 1.0f); // Blue for down
    }

    // Draw arrow indicator (simple triangle or diamond)
    vertices.clear();
    float arrowSize = 0.012f;

    if (std::abs(navData_.altitudeDifference) < 50.0f)
    {
      // Draw diamond for level
      vertices.push_back(indicatorX - arrowSize);
      vertices.push_back(arrowY);
      vertices.push_back(indicatorX);
      vertices.push_back(arrowY + arrowSize);

      vertices.push_back(indicatorX);
      vertices.push_back(arrowY + arrowSize);
      vertices.push_back(indicatorX + arrowSize);
      vertices.push_back(arrowY);

      vertices.push_back(indicatorX + arrowSize);
      vertices.push_back(arrowY);
      vertices.push_back(indicatorX);
      vertices.push_back(arrowY - arrowSize);

      vertices.push_back(indicatorX);
      vertices.push_back(arrowY - arrowSize);
      vertices.push_back(indicatorX - arrowSize);
      vertices.push_back(arrowY);
    }
    else if (navData_.altitudeDifference > 0.0f)
    {
      // Up arrow
      glm::vec2 tip(indicatorX, arrowY - arrowSize);
      glm::vec2 left(indicatorX - arrowSize * 0.8f, arrowY + arrowSize * 0.5f);
      glm::vec2 right(indicatorX + arrowSize * 0.8f, arrowY + arrowSize * 0.5f);

      vertices.push_back(tip.x);
      vertices.push_back(tip.y);
      vertices.push_back(left.x);
      vertices.push_back(left.y);

      vertices.push_back(tip.x);
      vertices.push_back(tip.y);
      vertices.push_back(right.x);
      vertices.push_back(right.y);

      vertices.push_back(left.x);
      vertices.push_back(left.y);
      vertices.push_back(right.x);
      vertices.push_back(right.y);
    }
    else
    {
      // Down arrow
      glm::vec2 tip(indicatorX, arrowY + arrowSize);
      glm::vec2 left(indicatorX - arrowSize * 0.8f, arrowY - arrowSize * 0.5f);
      glm::vec2 right(indicatorX + arrowSize * 0.8f, arrowY - arrowSize * 0.5f);

      vertices.push_back(tip.x);
      vertices.push_back(tip.y);
      vertices.push_back(left.x);
      vertices.push_back(left.y);

      vertices.push_back(tip.x);
      vertices.push_back(tip.y);
      vertices.push_back(right.x);
      vertices.push_back(right.y);

      vertices.push_back(left.x);
      vertices.push_back(left.y);
      vertices.push_back(right.x);
      vertices.push_back(right.y);
    }

    // Draw arrow
    if (!vertices.empty())
    {
      glBindBuffer(GL_ARRAY_BUFFER, vbo_);
      glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(float), vertices.data());

      shader_->setVec4("color", arrowColor);
      glLineWidth(3.0f);
      glDrawArrays(GL_LINES, 0, vertices.size() / 2);
    }
  }

  void WaypointIndicator::drawCircle(const glm::vec2 &center, float radius, const glm::vec4 &color)
  {
    const int segments = 64;
    std::vector<float> vertices;

    for (int i = 0; i <= segments; ++i)
    {
      float angle = 2.0f * M_PI * i / segments;
      float x = center.x + radius * std::cos(angle);
      float y = center.y + radius * std::sin(angle);
      vertices.push_back(x);
      vertices.push_back(y);
    }

    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(float), vertices.data());

    shader_->setVec4("color", color);

    glLineWidth(2.0f);
    glDrawArrays(GL_LINE_LOOP, 0, vertices.size() / 2);
  }

  float WaypointIndicator::normalizeAngle(float angle)
  {
    while (angle > 180.0f)
      angle -= 360.0f;
    while (angle < -180.0f)
      angle += 360.0f;
    return angle;
  }

  glm::vec2 WaypointIndicator::getCompassCenter() const
  {
    return position_ + glm::vec2(size_.x * 0.6f, size_.y * 0.5f);
  }

} // namespace hud
