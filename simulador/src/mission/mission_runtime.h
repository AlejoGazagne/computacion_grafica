/**
 * @file mission_runtime.h
 * @brief Gestión del estado de ejecución de una misión
 */

#pragma once

#include "mission_definition.h"
#include <chrono>
#include <string>

namespace hud
{
  struct FlightData;
}

namespace Mission
{

  /**
   * @brief Fases de una misión
   */
  enum class MissionPhase
  {
    Briefing,   // Mostrando briefing inicial, físicas pausadas
    InProgress, // Misión en curso, físicas activas
    Completed,  // Misión completada, esperando decisión del piloto
    FreeFlight  // Vuelo libre post-misión, sin waypoints
  };

  /**
   * @brief Contexto de inicio de misión
   */
  struct MissionStartContext
  {
    int countdownSeconds = 3;
    bool showBriefing = true;
    glm::vec3 startPosition;
    glm::quat startOrientation;
    float recommendedSpeed = 150.0f;     // kt
    float recommendedAltitude = 1500.0f; // ft
  };

  /**
   * @brief Métricas de rendimiento de la misión
   */
  struct MissionMetrics
  {
    float totalTimeSeconds = 0.0f;
    int waypointsCaptured = 0;
    int totalWaypoints = 0;
    float averageSpeed = 0.0f; // kt
    float maxAltitude = 0.0f;  // ft
    bool perfectRun = true;    // Sin errores graves
  };

  /**
   * @brief Gestor del estado de ejecución de misiones
   */
  class MissionRuntime
  {
  public:
    MissionRuntime();
    ~MissionRuntime() = default;

    // CONTROL DE MISIÓN
    MissionStartContext startMission(const MissionDefinition &mission);
    void markWaypointCaptured(int waypointIndex);
    void markCompletion();
    void continueFreeFlight();
    void requestMenuExit();
    void reset();

    // ACTUALIZACIÓN DE PROGRESO
    void updateProgress(const hud::FlightData &flightData, float dt);
    void updateMetrics(const hud::FlightData &flightData, float dt);

    // CONSULTAS DE ESTADO
    MissionPhase phase() const { return phase_; }
    bool hasMission() const { return hasMission_; }
    bool isCompleted() const { return phase_ == MissionPhase::Completed || phase_ == MissionPhase::FreeFlight; }
    bool areWaypointsEnabled() const
    {
      return hasMission_ && (phase_ == MissionPhase::InProgress || phase_ == MissionPhase::Briefing);
    }
    bool shouldRunPhysics() const
    {
      return phase_ == MissionPhase::InProgress || phase_ == MissionPhase::FreeFlight;
    }
    bool shouldShowOverlay() const
    {
      return phase_ == MissionPhase::Briefing || phase_ == MissionPhase::Completed;
    }
    bool menuExitRequested() const { return menuExitRequested_; }
    int getActiveWaypointIndex() const { return activeWaypointIndex_; }
    const MissionDefinition &getMission() const { return currentMission_; }
    const MissionMetrics &getMetrics() const { return metrics_; }
    void confirmReadyToFly();

  private:
    // Estado de la misión
    MissionPhase phase_;
    bool hasMission_;
    MissionDefinition currentMission_;
    bool menuExitRequested_;

    // Waypoints y progreso
    int activeWaypointIndex_;
    std::vector<bool> waypointsCaptured_;

    // Métricas acumuladas
    MissionMetrics metrics_;
    std::chrono::steady_clock::time_point missionStartTime_;
    float speedAccumulator_; // Para calcular velocidad promedio

    // Helper
    bool checkWaypointCapture(const glm::vec3 &planePos, const glm::vec3 &waypointPos, float captureRadius = 100.0f);
  };

} // namespace Mission
