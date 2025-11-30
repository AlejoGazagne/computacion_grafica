/**
 * @file waypoint_system.cpp
 * @brief Implementación del sistema de waypoints
 */

#include "waypoint_system.h"
#include "waypoint_renderer.h"
#include <iostream>
#include <glm/gtc/constants.hpp>

extern "C"
{
#include <glad/glad.h>
}

namespace Mission
{

  WaypointSystem::WaypointSystem() = default;
  WaypointSystem::~WaypointSystem() = default;

  void WaypointSystem::initialize()
  {
    renderer_ = std::make_unique<WaypointRenderer>();
    renderer_->initialize();
    std::cout << "WaypointSystem initialized successfully" << std::endl;
  }

  void WaypointSystem::cleanup()
  {
    if (renderer_)
    {
      renderer_->cleanup();
    }
  }

  void WaypointSystem::reset()
  {
    waypoints_.clear();
  }

  void WaypointSystem::loadFromMission(const MissionDefinition &mission)
  {
    reset();
    waypoints_.reserve(mission.waypoints.size());
    for (const auto &wp : mission.waypoints)
    {
      waypoints_.push_back({wp.position, wp.name, false});
    }

    if (!mission.environment.timeOfDay.empty())
    {
      std::cout << "Hora: " << mission.environment.timeOfDay << std::endl;
    }
    if (!mission.environment.weather.empty())
    {
      std::cout << "Clima: " << mission.environment.weather << std::endl;
    }
  }

  void WaypointSystem::update(const glm::vec3 &planePos,
                              hud::FlightData &flightData,
                              MissionRuntime &runtime)
  {
    if (!runtime.areWaypointsEnabled() || waypoints_.empty())
    {
      flightData.hasActiveWaypoint = false;
      return;
    }

    // Buscar siguiente waypoint no capturado
    int nextWaypointIndex = -1;
    for (size_t i = 0; i < waypoints_.size(); ++i)
    {
      if (!waypoints_[i].captured)
      {
        nextWaypointIndex = static_cast<int>(i);
        break;
      }
    }

    if (nextWaypointIndex == -1)
    {
      // Todos los waypoints fueron capturados
      flightData.hasActiveWaypoint = false;
      return;
    }

    const auto &currentWaypoint = waypoints_[nextWaypointIndex];
    flightData.targetWaypoint = currentWaypoint.position;
    flightData.hasActiveWaypoint = true;

    // Vector hacia el próximo waypoint
    glm::vec3 toWaypoint = currentWaypoint.position - planePos;
    flightData.waypointDistance = glm::length(toWaypoint);

    // Calcular bearing proyectando en el plano XZ
    glm::vec2 toWaypointXZ = glm::vec2(toWaypoint.x, toWaypoint.z);
    if (glm::length(toWaypointXZ) > 0.01f)
    {
      float bearing = atan2(toWaypointXZ.x, -toWaypointXZ.y) * (180.0f / glm::pi<float>());
      if (bearing < 0.0f)
      {
        bearing += 360.0f;
      }
      flightData.waypointBearing = bearing;
    }

    const float kCaptureRadius = 100.0f;
    if (flightData.waypointDistance < kCaptureRadius)
    {
      // Marcar captura local y notificar al runtime
      waypoints_[nextWaypointIndex].captured = true;
      runtime.markWaypointCaptured(nextWaypointIndex);

      // Contar cuántos faltan
      int remaining = 0;
      for (const auto &wp : waypoints_)
      {
        if (!wp.captured)
        {
          remaining++;
        }
      }

      std::cout << "✓ Waypoint " << currentWaypoint.name << " alcanzado! ";
      if (remaining > 0)
      {
        std::cout << "Waypoints restantes: " << remaining << std::endl;
      }
      else
      {
        std::cout << "¡Último waypoint!" << std::endl;
      }
    }
  }

  void WaypointSystem::render(const glm::mat4 &view,
                              const glm::mat4 &projection,
                              const MissionRuntime &runtime) const
  {
    if (!renderer_ || waypoints_.empty() || !runtime.areWaypointsEnabled())
    {
      return;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    for (size_t i = 0; i < waypoints_.size(); ++i)
    {
      if (waypoints_[i].captured)
      {
        continue;
      }

      int activeIdx = runtime.getActiveWaypointIndex();
      bool isActive = static_cast<int>(i) == activeIdx;
      glm::vec4 color = isActive
                            ? glm::vec4(0.0f, 1.0f, 0.4f, 0.8f)
                            : glm::vec4(0.2f, 0.5f, 1.0f, 0.6f);

      renderer_->drawWaypoint(view, projection, waypoints_[i].position, color, isActive);
    }

    glDisable(GL_BLEND);
  }

  void WaypointSystem::skipActiveWaypoint(MissionRuntime &runtime)
  {
    int activeIdx = runtime.getActiveWaypointIndex();
    if (activeIdx < 0 || activeIdx >= static_cast<int>(waypoints_.size()))
    {
      return;
    }

    if (!waypoints_[activeIdx].captured)
    {
      waypoints_[activeIdx].captured = true;
      runtime.markWaypointCaptured(activeIdx);
      std::cout << "Waypoint " << waypoints_[activeIdx].name << " saltado manualmente" << std::endl;
    }
  }

} // namespace Mission
