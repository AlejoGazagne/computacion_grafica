#ifndef PITCH_LADDER_H
#define PITCH_LADDER_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
extern "C" {
    #include <glad/glad.h>
    #include <GLFW/glfw3.h>
}
#include <memory>
#include <vector>

#include "graphics/shaders/shader_manager.h"

namespace UI {

    /**
     * @brief Indicador de Pitch Ladder para simulador de vuelo
     * Muestra una mira central y líneas horizontales que representan diferentes ángulos de pitch
     */
    class PitchLadder {
    private:
        // Buffers para renderizado 2D
        GLuint VAO_, VBO_;
        std::string shader_name_;
        
        // Dimensiones de pantalla
        int screen_width_;
        int screen_height_;
        
        // Configuración del pitch ladder
        static constexpr int NUM_PITCH_LINES = 9;  // 9 líneas: -40°, -30°, -20°, -10°, 0°, 10°, 20°, 30°, 40°
        static constexpr float PITCH_STEP = 10.0f; // Cada línea representa 10° de diferencia
        static constexpr float MAX_PITCH_DISPLAY = 40.0f; // Mostrar hasta ±40°
        
        bool initializeOpenGL();
        void cleanup();
        
        // Métodos auxiliares para generar geometría
        void generateCrosshairVertices(std::vector<float>& vertices, float center_x, float center_y);
        void generatePitchLineVertices(std::vector<float>& vertices, float pitch_angle, float camera_pitch);

    public:
        PitchLadder(int width, int height, const std::string& shader_name = "pitch_ladder_shader");
        ~PitchLadder();
        
        void updateScreenSize(int width, int height);
        void render(float camera_pitch);
        
        bool isInitialized() const { return VAO_ != 0; }
    };

} // namespace UI

#endif // PITCH_LADDER_H
