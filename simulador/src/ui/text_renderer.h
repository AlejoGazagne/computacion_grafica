#ifndef TEXT_RENDERER_H
#define TEXT_RENDERER_H

#include <vector>
#include <string>

namespace UI {

    /**
     * @brief Renderizador de texto simple usando líneas para HUDs de aviación
     * Genera vértices para números utilizando líneas OpenGL
     */
    class TextRenderer {
    public:
        /**
         * @brief Genera vértices para renderizar un número usando líneas
         * @param number El número a renderizar
         * @param x Posición X del texto
         * @param y Posición Y del texto
         * @param digit_width Ancho de cada dígito
         * @param digit_height Altura de cada dígito
         * @return Vector de vértices para renderizar con GL_LINES
         */
        static std::vector<float> generateNumberVertices(
            int number, 
            float x, 
            float y, 
            float digit_width = 0.02f, 
            float digit_height = 0.03f
        );

    private:
        /**
         * @brief Genera vértices para un dígito individual
         * @param digit Dígito del 0 al 9
         * @param x Posición X del dígito
         * @param y Posición Y del dígito
         * @param width Ancho del dígito
         * @param height Altura del dígito
         * @return Vector de vértices para el dígito
         */
        static std::vector<float> generateDigitVertices(
            int digit, 
            float x, 
            float y, 
            float width, 
            float height
        );
    };

} // namespace UI

#endif // TEXT_RENDERER_H