#ifndef BANK_ANGLE_H
#define BANK_ANGLE_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
extern "C"
{
#include <glad/glad.h>
#include <GLFW/glfw3.h>
}
#include <memory>
#include <vector>

#include "graphics/shaders/shader_manager.h"
#include "hud/instrumentbase.h"
#include "hud/huddef.h"

namespace UI
{

    /**
     * @brief Indicador de Bank Angle para simulador de vuelo
     * Muestra la inclinación lateral del avión en la parte inferior de la pantalla
     */
    class BankAngleIndicator : public hud::InstrumentBase
    {
    private:
        // Estado
        float bank_angle_deg_ = 0.0f; // [deg]

        bool initializeOpenGL();
        void cleanup();

    protected:
        void updateModelMatrix() override;

    public:
        BankAngleIndicator(const glm::vec2 &pos,
                           const glm::vec2 &size,
                           Graphics::Shaders::Shader *shader);
        ~BankAngleIndicator();

        // Ciclo de vida
        void initialize() override;                        // crea VAO/VBO y configura atributos
        void update(const hud::FlightData &data) override; // toma datos de vuelo
        void render() override;                            // dibuja el instrumento
    };

} // namespace UI

#endif // BANK_ANGLE_H