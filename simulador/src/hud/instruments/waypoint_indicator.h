#ifndef WAYPOINT_INDICATOR_H
#define WAYPOINT_INDICATOR_H

#include "../../../include/hud/instrumentbase.h"
#include "graphics/rendering/buffer_objects.h"
#include <glm/glm.hpp>
#include <memory>

namespace hud
{

  /**
   * @class WaypointIndicator
   * @brief HSI (Horizontal Situation Indicator) compass showing direction to active waypoint
   *
   * Displays a compass rose with:
   * - Fixed rose with tick marks every 5° and labels every 30°
   * - Cardinal direction labels (N, E, S, W)
   * - Magenta arrow pointing to the active waypoint
   * - Vertical altitude difference indicator (UP/DN/LVL)
   */
  class WaypointIndicator : public InstrumentBase
  {
  public:
    /**
     * @brief Constructor
     * @param pos NDC position (-1 to 1)
     * @param size NDC size
     * @param shader Pointer to HUD shader
     */
    WaypointIndicator(const glm::vec2 &pos, const glm::vec2 &size, Graphics::Shaders::Shader *shader);

    ~WaypointIndicator() override;

    void initialize() override;
    void update(const FlightData &data) override;
    void render() override;
    void updateModelMatrix() override {}

  private:
    struct NavData
    {
      float heading = 0.0f;            // Current aircraft heading (0-360°)
      float relativeAngle = 0.0f;      // Relative angle to waypoint (-180 to +180°)
      float altitudeDifference = 0.0f; // Altitude difference in meters
      bool hasWaypoint = false;        // Whether there's an active waypoint
    };

    NavData navData_;

    // Resources
    std::unique_ptr<Graphics::Rendering::VertexArray> vertex_array_;
    Graphics::Rendering::VertexBuffer* vertex_buffer_;

    // Geometry configuration
    static constexpr float ROSE_RADIUS = 0.15f;   // Compass rose radius in NDC
    static constexpr float MAJOR_TICK = 0.025f;   // Major tick length (every 10°)
    static constexpr float MINOR_TICK = 0.012f;   // Minor tick length (every 5°)
    static constexpr float VERT_OFFSET = 0.08f;   // Vertical indicator offset
    static constexpr float VERT_HEIGHT = 0.20f;   // Vertical indicator height
    static constexpr float MAX_ALT_DIFF = 500.0f; // Max altitude difference shown

    // Rendering methods
    void renderCompassRose();
    void renderVerticalIndicator();
    void renderCompassTicks();
    void renderWaypointPointer();
    void drawCircle(const glm::vec2 &center, float radius, const glm::vec4 &color);

    // Utility methods
    static float normalizeAngle(float angle);
    glm::vec2 getCompassCenter() const;
  };

} // namespace hud

#endif // WAYPOINT_INDICATOR_H
