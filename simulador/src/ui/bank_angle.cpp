#include "bank_angle.h"
#include "text_renderer.h"
#include <iostream>
#include <cmath>
#include <string>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace UI
{

    BankAngleIndicator::BankAngleIndicator(const glm::vec2 &pos,
                                           const glm::vec2 &size,
                                           Graphics::Shaders::Shader *shader)
        : hud::InstrumentBase(pos, size, shader)
    {
    }

    BankAngleIndicator::~BankAngleIndicator()
    {
        cleanup();
    }

    bool BankAngleIndicator::initializeOpenGL()
    {
        // Crear VAO y VBO usando los del InstrumentBase
        glGenVertexArrays(1, &vao_);
        glGenBuffers(1, &vbo_);

        glBindVertexArray(vao_);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_);
        // Reservar buffer vacío inicial
        glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

        // Configurar atributo de posición
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);

        glBindVertexArray(0);
        return true;
    }

    void BankAngleIndicator::cleanup()
    {
        clean(); // usa clean_instrument + clean_buffers del base
    }

    void BankAngleIndicator::initialize()
    {
        if (!initializeOpenGL())
        {
            std::cerr << "Failed to initialize BankAngleIndicator OpenGL resources" << std::endl;
        }
    }

    void BankAngleIndicator::update(const hud::FlightData &data)
    {
        bank_angle_deg_ = data.roll; // grados
        updateModelMatrix();
    }

    void BankAngleIndicator::render()
    {
        auto *shader = shader_;
        if (!shader || !shader->isCompiled())
            return;

        // Normalizar el ángulo de roll para aviación: 0° a 180° y luego cuenta regresiva
        float normalized_angle = bank_angle_deg_;

        // Normalizar a rango -180° a 180°
        while (normalized_angle > 180.0f)
            normalized_angle -= 360.0f;
        while (normalized_angle < -180.0f)
            normalized_angle += 360.0f;

        // Usar el ángulo normalizado para todos los cálculos
        float display_angle = normalized_angle;

        // Guardar estado GL
        GLboolean depthWasEnabled = glIsEnabled(GL_DEPTH_TEST);

        // Configurar OpenGL para renderizado 2D
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        shader->use();
        glBindVertexArray(vao_);

        // Color del indicador (verde para las marcas)
        shader->setVec3("color", 0.0f, 1.0f, 0.0f);

        // === INDICADOR DE BANK ANGLE - LÍNEA RECTA CON INCLINACIÓN ===

        float center_x = 0.0f;
        float center_y = -0.85f;  // Posición vertical del HUD
        float line_width = 0.4f;  // Ancho total de la línea
        float line_slope = 0.05f; // Ligera inclinación hacia arriba (5% de pendiente)

        // Línea recta con ligera inclinación: va desde abajo-izquierda a arriba-derecha
        float left_x = center_x - line_width / 2;
        float left_y = center_y - line_slope * line_width / 2; // Más abajo a la izquierda
        float right_x = center_x + line_width / 2;
        float right_y = center_y + line_slope * line_width / 2; // Más arriba a la derecha

        // Líneas de graduación móviles - Cada línea representa 10° de inclinación
        float line_spacing = 0.045f;                                           // Espaciado aumentado para dar espacio a los números
        float degrees_per_line = 10.0f;                                        // Cada línea representa 10 grados
        float line_offset = (display_angle / degrees_per_line) * line_spacing; // Desplazamiento basado en grados

        // Calcular qué línea está en el centro basado en el ángulo actual
        int center_line_index = (int)round(display_angle / degrees_per_line);

        // Solo generar 5 líneas: 2 a la izquierda, 1 central, 2 a la derecha
        // Cada línea representa múltiplos de 10°: ...-20°, -10°, 0°, +10°, +20°...
        int visible_lines = 0;
        for (int i = center_line_index - 2; i <= center_line_index + 2; i++)
        {
            // Calcular el ángulo que representa esta línea
            float line_angle = i * degrees_per_line;

            // Posición a lo largo de la línea inclinada basada en la diferencia de ángulos
            float angle_diff = line_angle - display_angle;
            // Cambiar de resta a suma para que el movimiento sea en la misma dirección
            float t = 0.5f + (angle_diff / degrees_per_line) * (line_spacing / 0.4f); // Normalizar al ancho total

            // Solo dibujar líneas que estén dentro del rango visible de la línea base
            if (t >= 0.0f && t <= 1.0f)
            {
                // Interpolar posición en la línea inclinada
                float line_x = left_x + t * (right_x - left_x);
                float line_y = left_y + t * (right_y - left_y);

                // Altura de la línea de graduación
                // Líneas principales cada 30° (múltiplos de 3 líneas de 10°)
                bool is_major = (i % 3 == 0);                  // 0°, ±30°, ±60°, ±90°, etc.
                float mark_height = is_major ? 0.04f : 0.025f; // Mayor diferencia entre líneas principales y secundarias

                // Líneas verticales de graduación
                std::vector<float> mark = {
                    line_x, line_y - mark_height / 2,
                    line_x, line_y + mark_height / 2};

                glBindBuffer(GL_ARRAY_BUFFER, vbo_);
                glBufferData(GL_ARRAY_BUFFER, mark.size() * sizeof(float), mark.data(), GL_DYNAMIC_DRAW);
                glDrawArrays(GL_LINES, 0, 2);

                // Mostrar texto de grados cada 20° (múltiplos de 2 líneas de 10°)
                if (i % 2 == 0 && line_angle != 0.0f)
                { // Cada 20°, excluyendo 0°
                    // Usar TextRenderer para generar vértices del número
                    std::vector<float> text_vertices = TextRenderer::generateNumberVertices(
                        (int)line_angle,
                        line_x,
                        line_y + mark_height / 2 + 0.035f);

                    if (!text_vertices.empty())
                    {
                        glBindBuffer(GL_ARRAY_BUFFER, vbo_);
                        glBufferData(GL_ARRAY_BUFFER, text_vertices.size() * sizeof(float),
                                     text_vertices.data(), GL_DYNAMIC_DRAW);
                        glDrawArrays(GL_LINES, 0, text_vertices.size() / 2);
                    }
                }

                visible_lines++;
                if (visible_lines >= 5)
                    break; // Limitar a máximo 5 líneas
            }
        }

        // Triángulo fijo un poco más abajo de la línea inclinada
        float needle_x = center_x;
        float needle_y = center_y - 0.03f; // Desplazado hacia abajo

        // Mantener color verde más brillante para la aguja
        shader->setVec3("color", 0.0f, 1.0f, 0.2f);

        // Crear triángulo fijo apuntando hacia arriba
        float triangle_size = 0.020f;

        // Triángulo simple apuntando hacia arriba
        float tip_x = needle_x;
        float tip_y = needle_y + triangle_size;
        float base_x1 = needle_x - triangle_size * 0.6f;
        float base_y1 = needle_y - triangle_size * 0.3f;
        float base_x2 = needle_x + triangle_size * 0.6f;
        float base_y2 = needle_y - triangle_size * 0.3f;

        std::vector<float> needle = {
            tip_x, tip_y,     // Punta hacia el centro
            base_x1, base_y1, // Base izquierda
            base_x2, base_y2  // Base derecha
        };

        glBindBuffer(GL_ARRAY_BUFFER, vbo_);
        glBufferData(GL_ARRAY_BUFFER, needle.size() * sizeof(float), needle.data(), GL_DYNAMIC_DRAW);
        glDrawArrays(GL_LINE_LOOP, 0, 3); // Triángulo hueco usando LINE_LOOP

        // Restaurar estado OpenGL
        glBindVertexArray(0);
        glUseProgram(0);
        if (depthWasEnabled)
            glEnable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);
    }

    void BankAngleIndicator::updateModelMatrix()
    {
        // Para HUD 2D en NDC, usamos identidad (pos/size podrían usarse para desplazar/escala si se desea)
        model_matrix_ = glm::mat4(1.0f);
    }

} // namespace UI

/*
En realidad los parametros de vuelo, como el angulo de bank van a venir de otro lado no van a ser tomados de la cámara
no se como se van a tomar
*/