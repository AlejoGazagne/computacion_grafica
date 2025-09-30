#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

extern "C" {
    #include <GLFW/glfw3.h>
}

#include <memory>
#include <array>
#include <string>
#include <vector>

namespace Scene {

    enum class CameraMovement {
        FORWARD,
        BACKWARD,
        LEFT,
        RIGHT,
        UP,
        DOWN,
        ROLL_LEFT,
        ROLL_RIGHT
    };

    enum class CameraType {
        PERSPECTIVE,
        ORTHOGRAPHIC,
        FIRST_PERSON,
        ORBITAL
    };

    struct CameraConfig {
        // Posición y orientación inicial
        glm::vec3 position = glm::vec3(0.0f, 0.0f, 3.0f);
        glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
        
        // Parámetros de proyección
        float fov = 45.0f;
        float aspect_ratio = 16.0f / 9.0f;
        float near_plane = 0.1f;
        float far_plane = 100.0f;
        
        // Configuración de movimiento
        float movement_speed = 2.5f;
        float mouse_sensitivity = 0.1f;
        float zoom_sensitivity = 1.0f;
        
        // Límites
        float min_fov = 1.0f;
        float max_fov = 120.0f;
        float min_pitch = -89.0f;
        float max_pitch = 89.0f;
        
        CameraType type = CameraType::FIRST_PERSON;
    };

    class Camera {
    private:
        CameraConfig config_;
        
        // Vectores de cámara
        glm::vec3 position_;
        glm::vec3 front_;
        glm::vec3 up_;
        glm::vec3 right_;
        glm::vec3 world_up_;
        
        // Ángulos de Euler
        float yaw_;
        float pitch_;
        float roll_;
        
        // Matrices
        mutable glm::mat4 view_matrix_;
        mutable glm::mat4 projection_matrix_;
        
        // Control de mouse
        bool first_mouse_;
        float last_x_;
        float last_y_;
        
        // Estado
        mutable bool matrices_dirty_;
        
        // Orbital camera específico
        float orbit_distance_;
        glm::vec3 orbit_target_;
        
        void updateCameraVectors();
        void updateViewMatrix() const;
        void updateProjectionMatrix();

    public:
        Camera();
        Camera(const CameraConfig& config);
        Camera(const glm::vec3& position, const glm::vec3& target = glm::vec3(0.0f));
        ~Camera() = default;

        // Configuración
        void setConfig(const CameraConfig& config);
        const CameraConfig& getConfig() const { return config_; }
        
        void setType(CameraType type);
        CameraType getType() const { return config_.type; }

        // Posición y orientación
        void setPosition(const glm::vec3& position);
        void setTarget(const glm::vec3& target);
        void setUp(const glm::vec3& up);
        void setRotation(float yaw, float pitch);
        void setRotation(float yaw, float pitch, float roll);
        
        const glm::vec3& getPosition() const { return position_; }
        const glm::vec3& getFront() const { return front_; }
        const glm::vec3& getUp() const { return up_; }
        const glm::vec3& getRight() const { return right_; }
        float getYaw() const { return yaw_; }
        float getPitch() const { return pitch_; }
        float getRoll() const { return roll_; }

        // Proyección
        void setPerspective(float fov, float aspect_ratio, float near_plane, float far_plane);
        void setOrthographic(float left, float right, float bottom, float top, float near_plane, float far_plane);
        void setAspectRatio(float aspect_ratio);
        void setFOV(float fov);
        
        float getFOV() const { return config_.fov; }
        float getAspectRatio() const { return config_.aspect_ratio; }
        float getNearPlane() const { return config_.near_plane; }
        float getFarPlane() const { return config_.far_plane; }

        // Matrices
        const glm::mat4& getViewMatrix() const;
        const glm::mat4& getProjectionMatrix() const;
        glm::mat4 getViewProjectionMatrix() const;
        
        // Para skybox (sin traslación)
        glm::mat4 getViewMatrixNoTranslation();

        // Movimiento
        void processKeyboardInput(GLFWwindow* window, float delta_time);
        void processMouseMovement(double xpos, double ypos);
        void processMouseScroll(float yoffset);
        
        // Movimiento manual
        void move(CameraMovement direction, float delta_time);
        void rotate(float yaw_offset, float pitch_offset);
        void zoom(float offset);

        // Orbital camera
        void setOrbitTarget(const glm::vec3& target);
        void setOrbitDistance(float distance);
        void orbitAroundTarget(float yaw_offset, float pitch_offset);
        
        const glm::vec3& getOrbitTarget() const { return orbit_target_; }
        float getOrbitDistance() const { return orbit_distance_; }

        // Utilidades
        void lookAt(const glm::vec3& target);
        void reset();
        void resetToConfig();
        
        // Ray casting (para picking)
        glm::vec3 screenToWorldRay(float screen_x, float screen_y, float screen_width, float screen_height) const;
        
        // Frustum (para culling)
        bool isPointInFrustum(const glm::vec3& point) const;
        bool isSphereInFrustum(const glm::vec3& center, float radius) const;

        // Configuración de controles
        void setMovementSpeed(float speed) { config_.movement_speed = speed; }
        void setMouseSensitivity(float sensitivity) { config_.mouse_sensitivity = sensitivity; }
        void setZoomSensitivity(float sensitivity) { config_.zoom_sensitivity = sensitivity; }
        
        float getMovementSpeed() const { return config_.movement_speed; }
        float getMouseSensitivity() const { return config_.mouse_sensitivity; }
        float getZoomSensitivity() const { return config_.zoom_sensitivity; }
    };

    // Controlador de cámara para manejar múltiples cámaras
    class CameraController {
    private:
        std::vector<std::unique_ptr<Camera>> cameras_;
        size_t active_camera_index_;
        
        // Configuración global
        bool mouse_captured_;
        GLFWwindow* window_;

    public:
        CameraController();
        ~CameraController() = default;

        // Gestión de cámaras
        size_t addCamera(std::unique_ptr<Camera> camera);
        Camera* getCamera(size_t index);
        Camera* getActiveCamera();
        
        void setActiveCamera(size_t index);
        size_t getActiveCameraIndex() const { return active_camera_index_; }
        size_t getCameraCount() const { return cameras_.size(); }
        
        void removeCamera(size_t index);
        void clear();

        // Control de entrada
        void setWindow(GLFWwindow* window) { window_ = window; }
        void setMouseCaptured(bool captured);
        bool isMouseCaptured() const { return mouse_captured_; }
        
        void processInput(float delta_time);
        void mouseCallback(GLFWwindow* window, double xpos, double ypos);
        void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);

        // Configuraciones por defecto
        static CameraConfig getFirstPersonConfig();
        static CameraConfig getOrbitalConfig();
        static CameraConfig getOrthographicConfig();
    };

} // namespace Scene

#endif // CAMERA_H