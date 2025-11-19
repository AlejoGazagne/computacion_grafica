#ifndef HUDDEF_H
#define HUDDEF_H

#include <glm/glm.hpp>

namespace hud
{

    // Estructura para datos de puntos de navegación
    typedef struct Waypoint
    {
        float latitude; // [deg]
        float longitud; // [deg]
        float altitude; // [ft]
    } Waypoint;

    // Estructura para datos de vuelo
    typedef struct FlightData
    {
        float pitch;          // [deg]
        float roll;           // [deg]
        float heading;        // [deg]
        float altitude;       // [ft]
        float speed;          // [kt]
        float vertical_speed; // [ft/min]
        float airspeed;       // [kt] - velocidad del aire
        glm::vec3 position;   // Posición actual del avión en el mundo
        Waypoint waypoint;

        // Campos para sistema de waypoints de misiones
        bool hasActiveWaypoint = false;
        glm::vec3 targetWaypoint = glm::vec3(0.0f);
        float waypointDistance = 0.0f; // distancia al waypoint activo [m]
        float waypointBearing = 0.0f;  // bearing al waypoint [deg]
    } FlightData;

} // namespace hud

#endif // HUDDEF_H
