/**
 * @file mission_definition.h
 * @brief Definición de estructuras para misiones
 */

#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Mission
{

  /**
   * @brief Representación de un waypoint en una misión
   */
  struct WaypointDef
  {
    glm::vec3 position;
    std::string name;

    WaypointDef() : position(0.0f), name("") {}
    WaypointDef(glm::vec3 pos, std::string n) : position(pos), name(n) {}
  };

  /**
   * @brief Condiciones ambientales de una misión
   */
  struct EnvironmentSettings
  {
    std::string timeOfDay = "day"; // "day", "dusk", "night"
    std::string weather = "clear"; // "clear", "cloudy", "rain", "storm"
    float windSpeed = 0.0f;        // m/s
    float windDirection = 0.0f;    // degrees

    EnvironmentSettings() = default;
  };

  /**
   * @brief Definición completa de una misión
   */
  struct MissionDefinition
  {
    std::string id;          // Identificador único
    std::string name;        // Nombre mostrado
    std::string description; // Descripción corta
    std::string briefing;    // Briefing detallado
    std::vector<WaypointDef> waypoints;
    glm::vec3 startPosition;    // Posición inicial del avión
    glm::quat startOrientation; // Orientación inicial
    EnvironmentSettings environment;
    std::string thumbnailPath;           // Ruta opcional a imagen
    std::string category;                // "training", "combat", "recon", etc.
    int difficulty = 1;                  // 1-5
    float recommendedSpeed = 150.0f;     // kt - velocidad recomendada
    float recommendedAltitude = 5000.0f; // ft - altitud recomendada

    MissionDefinition()
        : id(""), name(""), description(""), briefing(""),
          startPosition(0.0f, 1500.0f, 0.0f),
          startOrientation(1.0f, 0.0f, 0.0f, 0.0f),
          category("training"), difficulty(1),
          recommendedSpeed(150.0f), recommendedAltitude(5000.0f) {}
  };

} // namespace Mission
