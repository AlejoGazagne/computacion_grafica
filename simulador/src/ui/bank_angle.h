#ifndef BANK_ANGLE_H
#define BANK_ANGLE_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <vector>

#include "hud_instrument.h"

namespace UI
{

    /**
     * @brief Indicador de Bank Angle para simulador de vuelo
     * Muestra la inclinación lateral del avión en la parte inferior de la pantalla
     *
     * Hereda de HUDInstrument para aprovechar la gestión común de recursos OpenGL.
     */
    class BankAngleIndicator : public HUDInstrument
    {
    private:
        // Dato actual del instrumento
        float bank_angle_;

    public:
        BankAngleIndicator(int width, int height, const std::string &shader_name = "bank_angle_shader");
        ~BankAngleIndicator() override = default;

        /**
         * @brief Actualiza el ángulo de inclinación lateral
         * @param bank_angle Ángulo de roll en grados
         */
        void setBankAngle(float bank_angle) { bank_angle_ = bank_angle; }

        /**
         * @brief Renderiza el indicador con el último bank angle configurado
         */
        void render() override;
    };

} // namespace UI

#endif // BANK_ANGLE_H