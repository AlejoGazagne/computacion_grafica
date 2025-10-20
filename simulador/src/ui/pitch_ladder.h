#ifndef PITCH_LADDER_H
#define PITCH_LADDER_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <vector>

#include "hud_instrument.h"

namespace UI
{

    /**
     * @brief Indicador de Pitch Ladder para simulador de vuelo
     * Muestra una mira central y líneas horizontales que representan diferentes ángulos de pitch
     *
     * Hereda de HUDInstrument para aprovechar la gestión común de recursos OpenGL.
     */
    class PitchLadder : public HUDInstrument
    {
    private:
        // Dato actual del instrumento
        float camera_pitch_;

        // Configuración del pitch ladder
        static constexpr int NUM_PITCH_LINES = 9;         // 9 líneas: -40°, -30°, -20°, -10°, 0°, 10°, 20°, 30°, 40°
        static constexpr float PITCH_STEP = 10.0f;        // Cada línea representa 10° de diferencia
        static constexpr float MAX_PITCH_DISPLAY = 40.0f; // Mostrar hasta ±40°

        // Métodos auxiliares para generar geometría
        void generateCrosshairVertices(std::vector<float> &vertices, float center_x, float center_y);
        void generatePitchLineVertices(std::vector<float> &vertices, float pitch_angle, float camera_pitch);

    public:
        PitchLadder(int width, int height, const std::string &shader_name = "pitch_ladder_shader");
        ~PitchLadder() override = default;

        /**
         * @brief Actualiza el ángulo de pitch de la cámara
         * @param camera_pitch Ángulo de pitch en grados
         */
        void setPitch(float camera_pitch) { camera_pitch_ = camera_pitch; }

        /**
         * @brief Renderiza el pitch ladder con el último pitch configurado
         */
        void render() override;
    };

} // namespace UI

#endif // PITCH_LADDER_H
