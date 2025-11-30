#ifndef HUD_PITCH_LADDER_H
#define HUD_PITCH_LADDER_H

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
     * @brief Indicador de Pitch Ladder
     * Muestra una mira central y líneas horizontales que representan diferentes ángulos de pitch
     */
    class PitchLadder : public hud::InstrumentBase
    {
    private:
        // Configuración del pitch ladder
        static constexpr int NUM_PITCH_LINES = 9;         // 9 líneas: -40°, -30°, -20°, -10°, 0°, 10°, 20°, 30°, 40°
        static constexpr float PITCH_STEP = 10.0f;        // Cada línea representa 10° de diferencia
        static constexpr float MAX_PITCH_DISPLAY = 40.0f; // Mostrar hasta ±40°
        float pitch_angle_deg_ = 0.0f;                    // [deg]

        // Resources
        std::unique_ptr<Graphics::Rendering::VertexArray> vertex_array_;
        Graphics::Rendering::VertexBuffer* vertex_buffer_;

        bool initializeOpenGL();
        void cleanup();

        // Métodos auxiliares para generar geometría
        void generateCrosshairVertices(std::vector<float> &vertices, float center_x, float center_y);
        void generatePitchLineVertices(std::vector<float> &vertices, float pitch_angle, float camera_pitch);

    public:
        PitchLadder(const glm::vec2 &pos,
                    const glm::vec2 &size,
                    Graphics::Shaders::Shader *shader);
        ~PitchLadder();

        // Ciclo de vida
        void initialize() override;
        void update(const hud::FlightData &data) override;
        void render() override;

    protected:
        void updateModelMatrix() override;
    };

} // namespace hud

#endif // HUD_PITCH_LADDER_H
