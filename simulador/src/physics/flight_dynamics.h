#ifndef FLIGHT_DYNAMICS_H
#define FLIGHT_DYNAMICS_H

#include <memory>
#include <glm/glm.hpp>
#include <dlfdm/fdmsolver.h>
#include <dlfdm/defines.h>

namespace Physics {

// Estructura para datos de puntos de navegación
typedef struct Waypoint {
    float latitude;     // [deg]
    float longitude;    // [deg]
    float altitude;     // [ft]
} Waypoint;

// Estructura para datos de vuelo
typedef struct FlightData {
    float pitch;           // [deg]
    float roll;            // [deg]
    float heading;         // [deg]
    float altitude;        // [ft]
    float speed;           // [kt]
    float vertical_speed;  // [ft/min]
    Waypoint waypoint;
} FlightData;

/**
 * @brief Clase que integra el modelo físico FDM con el simulador gráfico
 * 
 * Esta clase actúa como puente entre el modelo de dinámica de vuelo (FDM)
 * y el sistema de renderizado, proporcionando una interfaz simple para
 * obtener datos de vuelo actualizados.
 */
class FlightDynamicsManager {
public:
    FlightDynamicsManager();
    ~FlightDynamicsManager() = default;

    /**
     * @brief Inicializa el modelo físico con parámetros de un avión jet trainer
     */
    void initialize();

    /**
     * @brief Actualiza la simulación física
     * @param delta_time Tiempo transcurrido desde el último frame [s]
     */
    void update(float delta_time);

    /**
     * @brief Obtiene los datos de vuelo actuales
     * @return Estructura FlightData con todos los parámetros de vuelo
     */
    FlightData getFlightData() const;

    /**
     * @brief Obtiene la posición del avión en coordenadas del mundo
     * @return Vector 3D con la posición [x, y, z] en metros
     */
    glm::vec3 getPosition() const;

    /**
     * @brief Obtiene los ángulos de Euler (orientación del avión)
     * @return Vector 3D con [pitch, yaw, roll] en grados
     */
    glm::vec3 getEulerAngles() const;

    /**
     * @brief Obtiene la velocidad del avión
     * @return Velocidad en nudos (knots)
     */
    float getSpeed() const;

    /**
     * @brief Obtiene la altitud del avión
     * @return Altitud en pies (feet)
     */
    float getAltitude() const;

    /**
     * @brief Obtiene la matriz de modelo para renderizado
     * @return Matriz 4x4 de transformación
     */
    glm::mat4 getModelMatrix() const;

    /**
     * @brief Establece el estado inicial del avión
     * @param position Posición inicial [x, y, z] en metros
     * @param velocity Velocidad inicial [u, v, w] en m/s
     * @param euler Ángulos de Euler iniciales [pitch, yaw, roll] en grados
     */
    void setInitialState(const glm::vec3& position, const glm::vec3& velocity, const glm::vec3& euler);

    /**
     * @brief Establece las entradas de control (throttle, elevator, aileron, rudder)
     * @param controls Estructura con las entradas de control
     */
    void setControls(const dlfdm::ControlInputs& controls);

    /**
     * @brief Obtiene los controles actuales
     */
    dlfdm::ControlInputs& getControls() { return current_controls_; }
    const dlfdm::ControlInputs& getControls() const { return current_controls_; }

    /**
     * @brief Modifica el throttle (potencia del motor)
     * @param delta Cambio en throttle (se clampea entre 0 y 1)
     */
    void adjustThrottle(float delta);

    /**
     * @brief Modifica el elevator (control de pitch)
     * @param delta Cambio en radianes (se clampea a límites del avión)
     */
    void adjustElevator(float delta);

    /**
     * @brief Modifica el aileron (control de roll)
     * @param delta Cambio en radianes (se clampea a límites del avión)
     */
    void adjustAileron(float delta);

    /**
     * @brief Modifica el rudder (control de yaw)
     * @param delta Cambio en radianes (se clampea a límites del avión)
     */
    void adjustRudder(float delta);

    /**
     * @brief Obtiene el solver FDM subyacente (para acceso directo si es necesario)
     */
    dlfdm::FDMSolver& getFDMSolver() { return *fdm_solver_; }

private:
    std::unique_ptr<dlfdm::FDMSolver> fdm_solver_;
    dlfdm::AircraftParameters aircraft_params_;
    dlfdm::ControlInputs current_controls_;

    // Constantes de conversión
    static constexpr float METERS_TO_FEET = 3.28084f;
    static constexpr float MPS_TO_KNOTS = 1.94384f;
    static constexpr float RAD_TO_DEG = 57.2957795f;

    /**
     * @brief Carga los parámetros de un avión jet trainer (AERMACCHI S-211)
     */
    dlfdm::AircraftParameters loadJetTrainerModel();

    /**
     * @brief Convierte coordenadas NED (North-East-Down) a coordenadas del mundo OpenGL
     * @param ned_position Posición en sistema NED
     * @return Posición en sistema OpenGL (X-right, Y-up, Z-back)
     */
    glm::vec3 nedToWorldCoordinates(const glm::vec3& ned_position) const;
};

} // namespace Physics

#endif // FLIGHT_DYNAMICS_H
