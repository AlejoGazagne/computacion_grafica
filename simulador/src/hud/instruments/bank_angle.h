#ifndef HUD_BANK_ANGLE_H
#define HUD_BANK_ANGLE_H

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
#include "graphics/rendering/buffer_objects.h"
#include "hud/instrumentbase.h"
#include "hud/huddef.h"

namespace hud
{

    /**
     * @brief Indicador de Bank Angle
     * Muestra la inclinación lateral del avión en la parte inferior de la pantalla
     */
    class BankAngleIndicator : public hud::InstrumentBase
    {
    private:
        // Estado
        float bank_angle_deg_ = 0.0f; // [deg]

        // Recursos OpenGL gestionados por RAII
        std::unique_ptr<Graphics::Rendering::VertexArray> vertex_array_;
        Graphics::Rendering::VertexBuffer* vertex_buffer_; // Referencia observadora (propiedad del VAO)

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

} // namespace hud

#endif // HUD_BANK_ANGLE_H