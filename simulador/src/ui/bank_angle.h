#ifndef BANK_ANGLE_H
#define BANK_ANGLE_H

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
     * @brief Indicador de Bank Angle para simulador de vuelo
     * Muestra la inclinación lateral del avión en la parte inferior de la pantalla
     */
    class BankAngleIndicator {
    private:
        // Buffers para renderizado 2D
        GLuint VAO_, VBO_;
        std::string shader_name_;
        
        // Dimensiones de pantalla
        int screen_width_;
        int screen_height_;
        
        bool initializeOpenGL();
        void cleanup();


    public:
        BankAngleIndicator(int width, int height, const std::string& shader_name = "bank_angle_shader");
        ~BankAngleIndicator();
        
        void updateScreenSize(int width, int height);
        void render(float bank_angle);
        
        bool isInitialized() const { return VAO_ != 0; }
    };

} // namespace UI

#endif // BANK_ANGLE_H