/**
 * @file waypoint_system.h
 * @brief Administra los waypoints de la misión y su representación visual
 */

#pragma once

#include <memory>
#include <vector>
#include <string>
#include <glm/glm.hpp>

#include "hud/huddef.h"
#include "mission_definition.h"
#include "mission_runtime.h"

namespace Mission
{
  class WaypointRenderer;

  /**
   * @brief Administra los waypoints de la misión y su representación visual.
   */
  class WaypointSystem
  {
  public:
    WaypointSystem();
    ~WaypointSystem();

    /**
     * @brief Reserva recursos del renderer y deja el sistema listo.
     */
    void initialize();

    /**
     * @brief Limpia recursos.
     */
    void cleanup();

    /**
     * @brief Limpia la lista y marca todos los waypoints como no capturados.
     */
    void reset();

    /**
     * @brief Carga waypoints desde la definición de misión activa.
     */
    void loadFromMission(const MissionDefinition &mission);

    /**
     * @brief Calcula distancias, captura automática y actualiza FlightData.
     */
    void update(const glm::vec3 &planePos,
                hud::FlightData &flightData,
                MissionRuntime &runtime);

    /**
     * @brief Dibuja los marcadores 3D del waypoint activo (y próximos).
     */
    void render(const glm::mat4 &view,
                const glm::mat4 &projection,
                const MissionRuntime &runtime) const;

    /**
     * @brief Marca manualmente el waypoint activo como completado.
     */
    void skipActiveWaypoint(MissionRuntime &runtime);

    bool empty() const { return waypoints_.empty(); }

  private:
    struct WaypointEntry
    {
      glm::vec3 position;
      std::string name;
      bool captured = false;
    };

    std::unique_ptr<WaypointRenderer> renderer_;
    std::vector<WaypointEntry> waypoints_;
  };

} // namespace Mission
