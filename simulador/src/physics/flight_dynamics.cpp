#include "flight_dynamics.h"
#include <dlfdm/fdmsolver.h>
#include <dlfdm/defines.h>
#include <dlfdm/aerodynamicsmodel.h>
#include <iostream>
#include <cmath>

namespace Physics {

FlightDynamicsManager::FlightDynamicsManager() {
    // Inicializar controles en posición neutra/trim
    current_controls_.throttle = 0.3202f;   // 32% throttle para vuelo nivelado
    current_controls_.elevator = -0.09024f;  // Elevador trimado
    current_controls_.aileron = 0.0f;
    current_controls_.rudder = 0.0f;
}

void FlightDynamicsManager::initialize() {
    // Cargar parámetros del avión
    aircraft_params_ = loadJetTrainerModel();
    
    // Crear el solver FDM con un timestep de 120 Hz
    fdm_solver_ = std::make_unique<dlfdm::FDMSolver>(aircraft_params_, 1.0f / 120.0f);
    
    // Configurar condiciones iniciales de trim (vuelo nivelado)
    dlfdm::AircraftState init_state;
    
    // Posición inicial: sistema NED (North, East, Down)
    // Iniciamos a 5000m de altitud
    init_state.intertial_position = glm::vec3(0.0f, 0.0f, -1000.0f);  // [m]
    
    // Velocidad inicial: vuelo nivelado a ~150 m/s
    init_state.boby_velocity = glm::vec3(149.998f, 0.0f, -0.36675f);  // [m/s]
    
    // Sin rotación inicial
    init_state.body_omega = glm::vec3(0.0f, 0.0f, 0.0f);
    
    // Actitud inicial: nivelado
    init_state.theta = 0.0f;  // pitch
    init_state.phi = 0.0f;    // roll
    init_state.psi = 0.0f;    // yaw
    
    fdm_solver_->setState(init_state);
    
    std::cout << "Flight Dynamics Manager initialized successfully" << std::endl;
    std::cout << "  Initial altitude: " << getAltitude() << " ft" << std::endl;
    std::cout << "  Initial speed: " << getSpeed() << " kts" << std::endl;
}

void FlightDynamicsManager::update(float delta_time) {
    if (!fdm_solver_) {
        std::cerr << "ERROR: FDM Solver not initialized!" << std::endl;
        return;
    }
    
    // El FDM usa su propio timestep interno (120 Hz)
    // Podemos llamar a update múltiples veces si delta_time es grande
    const float fdm_timestep = 1.0f / 120.0f;  // 120 Hz
    float remaining_time = delta_time;
    
    while (remaining_time > 0.0f) {
        float dt = std::min(remaining_time, fdm_timestep);
        fdm_solver_->update(current_controls_);
        remaining_time -= dt;
    }
}

FlightData FlightDynamicsManager::getFlightData() const {
    FlightData data;
    
    if (!fdm_solver_) {
        return data;  // Retornar datos vacíos si no hay solver
    }
    
    const dlfdm::AircraftState& state = fdm_solver_->getState();
    
    // Convertir ángulos de Euler de radianes a grados
    data.pitch = state.theta * RAD_TO_DEG;
    data.roll = state.phi * RAD_TO_DEG;
    data.heading = state.psi * RAD_TO_DEG;
    
    // Normalizar heading a [0, 360)
    while (data.heading < 0.0f) data.heading += 360.0f;
    while (data.heading >= 360.0f) data.heading -= 360.0f;
    
    // Calcular altitud en pies (z negativo en NED = altura sobre el suelo)
    data.altitude = -state.intertial_position.z * METERS_TO_FEET;
    
    // Calcular velocidad total en nudos
    glm::vec3 vel = state.boby_velocity;
    float speed_mps = glm::length(vel);
    data.speed = speed_mps * MPS_TO_KNOTS;
    
    // Calcular velocidad vertical en ft/min
    // En NED, z positivo es hacia abajo, entonces -w es la tasa de ascenso
    float vertical_speed_mps = -state.boby_velocity.z;
    data.vertical_speed = vertical_speed_mps * METERS_TO_FEET * 60.0f;  // convertir a ft/min
    
    // Waypoint (por ahora vacío, puede ser utilizado más adelante)
    data.waypoint.latitude = 0.0f;
    data.waypoint.longitude = 0.0f;
    data.waypoint.altitude = data.altitude;
    
    return data;
}

glm::vec3 FlightDynamicsManager::getPosition() const {
    if (!fdm_solver_) {
        return glm::vec3(0.0f);
    }
    
    const dlfdm::AircraftState& state = fdm_solver_->getState();
    return nedToWorldCoordinates(state.intertial_position);
}

glm::vec3 FlightDynamicsManager::getEulerAngles() const {
    if (!fdm_solver_) {
        return glm::vec3(0.0f);
    }
    
    const dlfdm::AircraftState& state = fdm_solver_->getState();
    
    // Convertir ángulos de NED a OpenGL
    // En NED: psi=0 apunta al Norte (X+)
    // En OpenGL: yaw=0 debería apuntar en la dirección inicial del movimiento
    
    // Ajustar el yaw para que sea consistente con la conversión de coordenadas
    // NED: North=X+, East=Y+
    // OpenGL: Right=X+, Up=Y+, Back=Z+
    // Necesitamos rotar 90 grados porque North en NED se convierte en -Z en OpenGL
    
    float pitch_deg = state.theta * RAD_TO_DEG;  // pitch se mantiene igual
    float yaw_deg = (state.psi * RAD_TO_DEG) - 90.0f;  // ajustar yaw: North(0°) -> -Z que es yaw=-90°
    float roll_deg = state.phi * RAD_TO_DEG;     // roll se mantiene igual
    
    // Normalizar yaw a [0, 360)
    while (yaw_deg < 0.0f) yaw_deg += 360.0f;
    while (yaw_deg >= 360.0f) yaw_deg -= 360.0f;
    
    // Retornar [pitch, yaw, roll] en grados
    return glm::vec3(pitch_deg, yaw_deg, roll_deg);
}

float FlightDynamicsManager::getSpeed() const {
    if (!fdm_solver_) {
        return 0.0f;
    }
    
    const dlfdm::AircraftState& state = fdm_solver_->getState();
    float speed_mps = glm::length(state.boby_velocity);
    return speed_mps * MPS_TO_KNOTS;
}

float FlightDynamicsManager::getAltitude() const {
    if (!fdm_solver_) {
        return 0.0f;
    }
    
    const dlfdm::AircraftState& state = fdm_solver_->getState();
    // En NED, z negativo es altitud
    return -state.intertial_position.z * METERS_TO_FEET;
}

glm::mat4 FlightDynamicsManager::getModelMatrix() const {
    if (!fdm_solver_) {
        return glm::mat4(1.0f);
    }
    
    return fdm_solver_->getModelMatrix();
}

void FlightDynamicsManager::setInitialState(const glm::vec3& position, 
                                           const glm::vec3& velocity, 
                                           const glm::vec3& euler) {
    if (!fdm_solver_) {
        std::cerr << "ERROR: Cannot set initial state - FDM not initialized" << std::endl;
        return;
    }
    
    dlfdm::AircraftState state;
    
    // Convertir posición de coordenadas del mundo a NED
    // OpenGL: X=right, Y=up, Z=back
    // NED: X=north, Y=east, Z=down
    state.intertial_position = glm::vec3(
        position.z,   // North = -Z en OpenGL
        position.x,   // East = X en OpenGL
        -position.y   // Down = -Y en OpenGL
    );
    
    state.boby_velocity = velocity;
    
    // Convertir ángulos de grados a radianes
    state.theta = euler.x / RAD_TO_DEG;  // pitch
    state.psi = euler.y / RAD_TO_DEG;    // yaw
    state.phi = euler.z / RAD_TO_DEG;    // roll
    
    state.body_omega = glm::vec3(0.0f);
    
    fdm_solver_->setState(state);
}

void FlightDynamicsManager::setControls(const dlfdm::ControlInputs& controls) {
    current_controls_ = controls;
}

void FlightDynamicsManager::adjustThrottle(float delta) {
    current_controls_.throttle += delta;
    // Clampear entre 0 y 1
    current_controls_.throttle = std::max(0.0f, std::min(1.0f, current_controls_.throttle));
}

void FlightDynamicsManager::adjustElevator(float delta) {
    current_controls_.elevator += delta;
    // Clampear a los límites del avión
    current_controls_.elevator = std::max(aircraft_params_.min_elevator, 
                                         std::min(aircraft_params_.max_elevator, current_controls_.elevator));
}

void FlightDynamicsManager::adjustAileron(float delta) {
    current_controls_.aileron += delta;
    // Clampear a los límites del avión
    current_controls_.aileron = std::max(aircraft_params_.min_aileron, 
                                        std::min(aircraft_params_.max_aileron, current_controls_.aileron));
}

void FlightDynamicsManager::adjustRudder(float delta) {
    current_controls_.rudder += delta;
    // Clampear a los límites del avión
    float max_rudder = aircraft_params_.max_rudder;
    current_controls_.rudder = std::max(-max_rudder, std::min(max_rudder, current_controls_.rudder));
}

dlfdm::AircraftParameters FlightDynamicsManager::loadJetTrainerModel() {
    dlfdm::AircraftParameters p;

    // Mass properties - AERMACCHI S-211
    p.mass = 1815.0f;           // [kg]
    p.Ixx = 1084.6f;            // [kg·m^2]
    p.Iyy = 6507.9f;            // [kg·m^2]
    p.Izz = 7050.2f;            // [kg·m^2]
    p.Ixz = 271.16f;            // [kg·m^2]

    // Aerodynamic reference
    p.wingArea = 12.63f;        // [m^2]
    p.wingChord = 1.64f;        // [m]
    p.wingSpan = 8.01f;         // [m]

    // Propulsion
    p.maxThrust = 11120.0f;     // [N]

    // Aerodynamic coefficients
    p.CL0 = 0.15f;              // [-]
    p.CLa = 5.5f;               // [1/rad]
    p.CL_delta_e = 0.38f;       // [1/rad]
    p.CD0 = 0.0205f;            // [-]
    p.CDa = 0.12f;              // [1/rad]
    p.Cm0 = -0.08f;             // [-]
    p.Cma = -0.24f;             // [1/rad]
    p.Cm_q = -15.7f;            // [1/rad]
    p.CY_beta = -1.0f;          // [1/rad]
    p.CY_r = 0.61f;             // [1/rad]
    p.CY_delta_r = 0.028f;      // [1/rad]
    p.Cl_beta = -0.11f;         // [1/rad]
    p.Cl_p = -0.39f;            // [1/rad]
    p.Cl_r = 0.28f;             // [1/rad]
    p.Cn_beta = 0.17f;          // [1/rad]
    p.Cn_p = 0.09f;             // [1/rad]
    p.Cn_r = -0.26f;            // [1/rad]

    // Control effectiveness
    p.Cm_delta_e = -0.88f;      // [1/rad]
    p.Cl_delta_a = 0.10f;       // [1/rad]
    p.Cn_delta_r = -0.12f;      // [1/rad]

    // Min-max surface deflections
    p.min_elevator = glm::radians(-15.0f);  // [rad]
    p.max_elevator = glm::radians(20.0f);   // [rad]
    p.min_aileron = glm::radians(-20.0f);   // [rad]
    p.max_aileron = glm::radians(20.0f);    // [rad]
    p.max_rudder = glm::radians(20.0f);     // [rad]

    return p;
}

glm::vec3 FlightDynamicsManager::nedToWorldCoordinates(const glm::vec3& ned_position) const {
    // Convertir de NED (North-East-Down) a OpenGL (X-right, Y-up, Z-back)
    // NED: X=north, Y=east, Z=down
    // OpenGL: X=right, Y=up, Z=back
    
    return glm::vec3(
        ned_position.y,   // X (right) = East
        -ned_position.z,  // Y (up) = -Down (invertir para que positivo sea arriba)
        -ned_position.x   // Z (back) = -North (invertir para que positivo sea hacia atrás)
    );
}

} // namespace Physics
