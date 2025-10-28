#include "camera.h"
#include <iostream>
#include <algorithm>
#include <cmath>

namespace Scene {

    // Constantes por defecto  
    const float DEFAULT_YAW = 180.0f;  // Mirar hacia Z negativo (hacia el cubo)
    const float DEFAULT_PITCH = 0.0f;

    // === Implementación de Camera ===

    Camera::Camera() 
        : config_(), position_(config_.position), yaw_(DEFAULT_YAW), pitch_(DEFAULT_PITCH), roll_(0.0f),
          first_mouse_(true), last_x_(0.0f), last_y_(0.0f), matrices_dirty_(true),
          orbit_distance_(5.0f), orbit_target_(0.0f) {
        
        world_up_ = config_.up;
        updateCameraVectors();
        updateViewMatrix();
        updateProjectionMatrix();
    }

    Camera::Camera(const CameraConfig& config)
        : config_(config), position_(config_.position), yaw_(DEFAULT_YAW), pitch_(DEFAULT_PITCH), roll_(0.0f),
          first_mouse_(true), last_x_(0.0f), last_y_(0.0f), matrices_dirty_(true),
          orbit_distance_(5.0f), orbit_target_(config_.target) {
        
        world_up_ = config_.up;
        
        // Calcular yaw y pitch basándose en target
        if (config_.type == CameraType::ORBITAL) {
            orbit_distance_ = glm::length(position_ - orbit_target_);
        }
        
        lookAt(config_.target);
        updateViewMatrix();
        updateProjectionMatrix();
    }

    Camera::Camera(const glm::vec3& position, const glm::vec3& target)
        : Camera() {
        position_ = position;
        config_.position = position;
        config_.target = target;
        orbit_target_ = target;
        orbit_distance_ = glm::length(position_ - target);
        
        lookAt(target);
        updateViewMatrix();
        updateProjectionMatrix();
    }

    void Camera::updateCameraVectors() {
        // Calcular el nuevo vector Front usando ángulos de Euler
        glm::vec3 front;
        front.x = cos(glm::radians(yaw_)) * cos(glm::radians(pitch_));
        front.y = sin(glm::radians(pitch_));
        front.z = sin(glm::radians(yaw_)) * cos(glm::radians(pitch_));
        front_ = glm::normalize(front);
        
        // Para permitir loops completos sin inversión de cámara,
        // usar un vector de referencia que dependa del yaw, no del world up
        glm::vec3 reference_right;
        reference_right.x = -sin(glm::radians(yaw_));
        reference_right.y = 0.0f;
        reference_right.z = cos(glm::radians(yaw_));
        reference_right = glm::normalize(reference_right);
        
        // Calcular Up vector base usando el reference_right
        glm::vec3 base_up = glm::normalize(glm::cross(reference_right, front_));
        
        // Aplicar roll (inclinación lateral) para simulador de vuelo
        glm::mat4 roll_matrix = glm::rotate(glm::mat4(1.0f), glm::radians(roll_), front_);
        up_ = glm::vec3(roll_matrix * glm::vec4(base_up, 0.0f));
        
        // Calcular right_ después del roll para mantener ortogonalidad
        right_ = glm::normalize(glm::cross(front_, up_));
        
        matrices_dirty_ = true;
    }

    void Camera::updateViewMatrix() const {
        if (config_.type == CameraType::ORBITAL) {
            view_matrix_ = glm::lookAt(position_, orbit_target_, up_);
        } else {
            view_matrix_ = glm::lookAt(position_, position_ + front_, up_);
        }
        matrices_dirty_ = false;
    }

    void Camera::updateProjectionMatrix() {
        projection_matrix_ = glm::perspective(
            glm::radians(config_.fov), 
            config_.aspect_ratio, 
            config_.near_plane, 
            config_.far_plane
        );
    }

    void Camera::setConfig(const CameraConfig& config) {
        config_ = config;
        position_ = config_.position;
        world_up_ = config_.up;
        orbit_target_ = config_.target;
        
        updateCameraVectors();
        updateProjectionMatrix();
        matrices_dirty_ = true;
    }

    void Camera::setType(CameraType type) {
        config_.type = type;
        
        if (type == CameraType::ORBITAL) {
            orbit_distance_ = glm::length(position_ - orbit_target_);
        }
        
        matrices_dirty_ = true;
    }

    void Camera::setPosition(const glm::vec3& position) {
        position_ = position;
        config_.position = position;
        
        if (config_.type == CameraType::ORBITAL) {
            orbit_distance_ = glm::length(position_ - orbit_target_);
        }
        
        matrices_dirty_ = true;
    }

    void Camera::setTarget(const glm::vec3& target) {
        config_.target = target;
        orbit_target_ = target;
        
        if (config_.type == CameraType::ORBITAL) {
            orbit_distance_ = glm::length(position_ - orbit_target_);
        }
        
        lookAt(target);
    }

    void Camera::setUp(const glm::vec3& up) {
        config_.up = up;
        world_up_ = up;
        updateCameraVectors();
    }

    void Camera::setRotation(float yaw, float pitch) {
        yaw_ = yaw;
        pitch_ = pitch;  // Sin restricciones para simulador de vuelo
        updateCameraVectors();
    }
    
    void Camera::setRotation(float yaw, float pitch, float roll) {
        yaw_ = yaw;
        pitch_ = pitch;  // Sin restricciones para simulador de vuelo
        roll_ = roll;
        updateCameraVectors();
    }

    void Camera::setPerspective(float fov, float aspect_ratio, float near_plane, float far_plane) {
        config_.fov = std::clamp(fov, config_.min_fov, config_.max_fov);
        config_.aspect_ratio = aspect_ratio;
        config_.near_plane = near_plane;
        config_.far_plane = far_plane;
        updateProjectionMatrix();
    }

    void Camera::setOrthographic(float left, float right, float bottom, float top, float near_plane, float far_plane) {
        config_.near_plane = near_plane;
        config_.far_plane = far_plane;
        projection_matrix_ = glm::ortho(left, right, bottom, top, near_plane, far_plane);
    }

    void Camera::setAspectRatio(float aspect_ratio) {
        config_.aspect_ratio = aspect_ratio;
        updateProjectionMatrix();
    }

    void Camera::setFOV(float fov) {
        config_.fov = std::clamp(fov, config_.min_fov, config_.max_fov);
        updateProjectionMatrix();
    }

    const glm::mat4& Camera::getViewMatrix() const {
        if (matrices_dirty_) {
            updateViewMatrix();
        }
        return view_matrix_;
    }

    const glm::mat4& Camera::getProjectionMatrix() const {
        return projection_matrix_;
    }

    glm::mat4 Camera::getViewProjectionMatrix() const {
        return getProjectionMatrix() * getViewMatrix();
    }

    glm::mat4 Camera::getViewMatrixNoTranslation() {
        if (matrices_dirty_) {
            updateViewMatrix();
        }
        // Remover la traslación para skybox
        return glm::mat4(glm::mat3(view_matrix_));
    }

    void Camera::processKeyboardInput(GLFWwindow* window, float delta_time) {
        float velocity = config_.movement_speed * delta_time;
        
        // Controles de simulador de vuelo
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            move(CameraMovement::FORWARD, delta_time);  // Solo W para acelerar adelante
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            move(CameraMovement::ROLL_LEFT, delta_time);  // A para inclinar izquierda
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            move(CameraMovement::ROLL_RIGHT, delta_time); // D para inclinar derecha
        
        // Mantener controles verticales para casos especiales
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            move(CameraMovement::DOWN, delta_time);
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
            move(CameraMovement::UP, delta_time);
    }

    void Camera::processMouseMovement(double xpos, double ypos) {
        if (first_mouse_) {
            last_x_ = static_cast<float>(xpos);
            last_y_ = static_cast<float>(ypos);
            first_mouse_ = false;
        }

        float xoffset = static_cast<float>(xpos) - last_x_;
        float yoffset = last_y_ - static_cast<float>(ypos); // Reversed since y-coordinates go from bottom to top
        
        last_x_ = static_cast<float>(xpos);
        last_y_ = static_cast<float>(ypos);

        xoffset *= config_.mouse_sensitivity;
        yoffset *= config_.mouse_sensitivity;

        if (config_.type == CameraType::ORBITAL) {
            orbitAroundTarget(xoffset, yoffset);
        } else {
            rotate(xoffset, yoffset);
        }
    }

    void Camera::processMouseScroll(float yoffset) {
        if (config_.type == CameraType::ORBITAL) {
            orbit_distance_ -= yoffset * config_.zoom_sensitivity;
            orbit_distance_ = std::max(orbit_distance_, 1.0f);
            
            // Actualizar posición orbital
            glm::vec3 direction = glm::normalize(position_ - orbit_target_);
            position_ = orbit_target_ + direction * orbit_distance_;
            matrices_dirty_ = true;
        } else {
            zoom(yoffset);
        }
    }

    void Camera::move(CameraMovement direction, float delta_time) {
        float velocity = config_.movement_speed * delta_time;
        float roll_speed = 45.0f; // Grados por segundo de roll
        
        switch (direction) {
            case CameraMovement::FORWARD:
                position_ += front_ * velocity;
                break;
            case CameraMovement::BACKWARD:
                position_ -= front_ * velocity;
                break;
            case CameraMovement::LEFT:
                position_ -= right_ * velocity;
                break;
            case CameraMovement::RIGHT:
                position_ += right_ * velocity;
                break;
            case CameraMovement::UP:
                position_ += up_ * velocity;
                break;
            case CameraMovement::DOWN:
                position_ -= up_ * velocity;
                break;
            case CameraMovement::ROLL_LEFT:
                roll_ -= roll_speed * delta_time;
                updateCameraVectors();
                break;
            case CameraMovement::ROLL_RIGHT:
                roll_ += roll_speed * delta_time;
                updateCameraVectors();
                break;
        }
        
        config_.position = position_;
        matrices_dirty_ = true;
    }

    void Camera::rotate(float yaw_offset, float pitch_offset) {
        yaw_ += yaw_offset;
        pitch_ += pitch_offset;
        
        // Sin restricciones de pitch para simulador de vuelo - permite loops completos
        // pitch_ = std::clamp(pitch_, config_.min_pitch, config_.max_pitch); // REMOVIDO
        
        updateCameraVectors();
    }

    void Camera::zoom(float offset) {
        config_.fov -= offset * config_.zoom_sensitivity;
        config_.fov = std::clamp(config_.fov, config_.min_fov, config_.max_fov);
        updateProjectionMatrix();
    }

    void Camera::setOrbitTarget(const glm::vec3& target) {
        orbit_target_ = target;
        config_.target = target;
        
        if (config_.type == CameraType::ORBITAL) {
            orbit_distance_ = glm::length(position_ - orbit_target_);
            matrices_dirty_ = true;
        }
    }

    void Camera::setOrbitDistance(float distance) {
        orbit_distance_ = std::max(distance, 0.1f);
        
        if (config_.type == CameraType::ORBITAL) {
            glm::vec3 direction = glm::normalize(position_ - orbit_target_);
            position_ = orbit_target_ + direction * orbit_distance_;
            config_.position = position_;
            matrices_dirty_ = true;
        }
    }

    void Camera::orbitAroundTarget(float yaw_offset, float pitch_offset) {
        yaw_ += yaw_offset;
        pitch_ += pitch_offset;
        
        // Sin restricciones de pitch para simulador de vuelo
        
        // Calcular nueva posición orbital
        float x = orbit_distance_ * cos(glm::radians(pitch_)) * cos(glm::radians(yaw_));
        float y = orbit_distance_ * sin(glm::radians(pitch_));
        float z = orbit_distance_ * cos(glm::radians(pitch_)) * sin(glm::radians(yaw_));
        
        position_ = orbit_target_ + glm::vec3(x, y, z);
        config_.position = position_;
        
        updateCameraVectors();
    }

    void Camera::lookAt(const glm::vec3& target) {
        glm::vec3 direction = glm::normalize(target - position_);  // Dirección HACIA el target
        
        // Calcular yaw (rotación horizontal)
        // atan2(z, x) da el ángulo en el plano XZ
        yaw_ = glm::degrees(atan2(direction.z, direction.x));
        
        // Calcular pitch (rotación vertical)
        pitch_ = glm::degrees(asin(direction.y));
        
        // Asegurar que pitch esté en el rango válido
        pitch_ = glm::clamp(pitch_, -89.0f, 89.0f);
        
        updateCameraVectors();
    }

    void Camera::reset() {
        resetToConfig();
    }

    void Camera::resetToConfig() {
        position_ = config_.position;
        yaw_ = DEFAULT_YAW;
        pitch_ = DEFAULT_PITCH;
        first_mouse_ = true;
        
        if (config_.type == CameraType::ORBITAL) {
            orbit_target_ = config_.target;
            orbit_distance_ = glm::length(position_ - orbit_target_);
            lookAt(orbit_target_);
        } else {
            updateCameraVectors();
        }
        
        matrices_dirty_ = true;
    }

    glm::vec3 Camera::screenToWorldRay(float screen_x, float screen_y, float screen_width, float screen_height) const {
        // Normalizar coordenadas de pantalla a NDC
        float x = (2.0f * screen_x) / screen_width - 1.0f;
        float y = 1.0f - (2.0f * screen_y) / screen_height;
        
        // Crear ray en clip space
        glm::vec4 ray_clip = glm::vec4(x, y, -1.0f, 1.0f);
        
        // Transform to eye space
        glm::mat4 inv_proj = glm::inverse(projection_matrix_);
        glm::vec4 ray_eye = inv_proj * ray_clip;
        ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0f, 0.0f);
        
        // Transform to world space
        glm::mat4 inv_view = glm::inverse(view_matrix_);
        glm::vec4 ray_world = inv_view * ray_eye;
        
        return glm::normalize(glm::vec3(ray_world));
    }

    bool Camera::isPointInFrustum(const glm::vec3& point) const {
        // Implementación básica de frustum culling
        glm::vec4 clip_space = getViewProjectionMatrix() * glm::vec4(point, 1.0f);
        
        if (clip_space.w <= 0) return false;
        
        glm::vec3 ndc = glm::vec3(clip_space) / clip_space.w;
        
        return (ndc.x >= -1.0f && ndc.x <= 1.0f &&
                ndc.y >= -1.0f && ndc.y <= 1.0f &&
                ndc.z >= -1.0f && ndc.z <= 1.0f);
    }

    bool Camera::isSphereInFrustum(const glm::vec3& center, float radius) const {
        // Verificar si la esfera está completamente fuera del frustum
        glm::vec4 clip_space = getViewProjectionMatrix() * glm::vec4(center, 1.0f);
        
        if (clip_space.w <= 0) return false;
        
        glm::vec3 ndc = glm::vec3(clip_space) / clip_space.w;
        
        // Aproximación simple: verificar si el centro está dentro del frustum extendido
        float extended_bound = 1.0f + radius; // Aproximación
        
        return (ndc.x >= -extended_bound && ndc.x <= extended_bound &&
                ndc.y >= -extended_bound && ndc.y <= extended_bound &&
                ndc.z >= -extended_bound && ndc.z <= extended_bound);
    }

    // === Implementación de CameraController ===

    CameraController::CameraController() 
        : active_camera_index_(0), mouse_captured_(false), window_(nullptr) {
    }

    size_t CameraController::addCamera(std::unique_ptr<Camera> camera) {
        cameras_.push_back(std::move(camera));
        return cameras_.size() - 1;
    }

    Camera* CameraController::getCamera(size_t index) {
        if (index < cameras_.size()) {
            return cameras_[index].get();
        }
        return nullptr;
    }

    Camera* CameraController::getActiveCamera() {
        return getCamera(active_camera_index_);
    }

    void CameraController::setActiveCamera(size_t index) {
        if (index < cameras_.size()) {
            active_camera_index_ = index;
        }
    }

    void CameraController::removeCamera(size_t index) {
        if (index < cameras_.size()) {
            cameras_.erase(cameras_.begin() + index);
            
            // Ajustar índice activo si es necesario
            if (active_camera_index_ >= cameras_.size() && !cameras_.empty()) {
                active_camera_index_ = cameras_.size() - 1;
            }
        }
    }

    void CameraController::clear() {
        cameras_.clear();
        active_camera_index_ = 0;
    }

    void CameraController::setMouseCaptured(bool captured) {
        mouse_captured_ = captured;
        
        if (window_) {
            if (captured) {
                glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            } else {
                glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            }
        }
    }

    void CameraController::processInput(float delta_time) {
        Camera* active_camera = getActiveCamera();
        if (active_camera && mouse_captured_ && window_) {
            active_camera->processKeyboardInput(window_, delta_time);
        }
    }

    void CameraController::mouseCallback(GLFWwindow* window, double xpos, double ypos) {
        Camera* active_camera = getActiveCamera();
        if (active_camera && mouse_captured_) {
            active_camera->processMouseMovement(xpos, ypos);
        }
    }

    void CameraController::scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
        Camera* active_camera = getActiveCamera();
        if (active_camera) {
            active_camera->processMouseScroll(static_cast<float>(yoffset));
        }
    }

    // Configuraciones por defecto
    CameraConfig CameraController::getFirstPersonConfig() {
        CameraConfig config;
        config.position = glm::vec3(0.0f, 5.0f, 10.0f);  // Posición elevada y alejada para ver el cubo y terreno
        config.target = glm::vec3(0.0f, 0.0f, 0.0f);  // Mirar al origen (donde está el cubo)
        config.type = CameraType::FIRST_PERSON;
        config.movement_speed = 50.0f;  // Velocidad más rápida para navegar el terreno grande
        config.mouse_sensitivity = 0.05f;  // Sensibilidad reducida para movimiento más suave
        config.far_plane = 100000.0f;  // Far plane extremadamente lejano
        return config;
    }

    CameraConfig CameraController::getOrbitalConfig() {
        CameraConfig config;
        config.position = glm::vec3(0.0f, 0.0f, 5.0f);
        config.target = glm::vec3(0.0f, 0.0f, 0.0f);
        config.type = CameraType::ORBITAL;
        config.movement_speed = 2.0f;
        config.mouse_sensitivity = 0.1f;  // Sensibilidad reducida para movimiento más suave
        return config;
    }

    CameraConfig CameraController::getOrthographicConfig() {
        CameraConfig config;
        config.position = glm::vec3(0.0f, 5.0f, 0.0f);
        config.target = glm::vec3(0.0f, 0.0f, 0.0f);
        config.type = CameraType::ORTHOGRAPHIC;
        config.fov = 60.0f;
        config.movement_speed = 3.0f;
        return config;
    }

} // namespace Scene