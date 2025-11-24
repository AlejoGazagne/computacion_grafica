#include "pitch_ladder.h"
#include <iostream>
#include <cmath>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace UI
{

    PitchLadder::PitchLadder(const glm::vec2 &pos,
                             const glm::vec2 &size,
                             Graphics::Shaders::Shader *shader)
        : hud::InstrumentBase(pos, size, shader)
    {
    }

    PitchLadder::~PitchLadder()
    {
        cleanup();
    }

    bool PitchLadder::initializeOpenGL()
    {
        // Crear VAO y VBO (usamos los del base)
        glGenVertexArrays(1, &vao_);
        glGenBuffers(1, &vbo_);

        glBindVertexArray(vao_);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_);

        // Reservar buffer vacío inicial
        glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

        // Configurar atributo de posición (solo coordenadas 2D)
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);

        glBindVertexArray(0);

        return true;
    }

    void PitchLadder::cleanup()
    {
        clean();
    }

    void PitchLadder::initialize()
    {
        if (!initializeOpenGL())
        {
            std::cerr << "Failed to initialize PitchLadder OpenGL resources" << std::endl;
        }
    }

    void PitchLadder::generateCrosshairVertices(std::vector<float> &vertices, float center_x, float center_y)
    {
        // Círculo pequeño en el centro
        float circle_radius = 0.01f; // Radio del círculo pequeño
        int circle_segments = 16;    // Segmentos para dibujar el círculo

        // Generar vértices del círculo
        for (int i = 0; i < circle_segments; ++i)
        {
            float angle1 = (i * 2.0f * M_PI) / circle_segments;
            float angle2 = ((i + 1) * 2.0f * M_PI) / circle_segments;

            vertices.push_back(center_x + circle_radius * cos(angle1));
            vertices.push_back(center_y + circle_radius * sin(angle1));
            vertices.push_back(center_x + circle_radius * cos(angle2));
            vertices.push_back(center_y + circle_radius * sin(angle2));
        }

        // Líneas laterales (saliendo del círculo hacia los lados)
        float line_length = 0.04f;         // Longitud de las líneas laterales
        float line_offset = circle_radius; // Empiezan desde el borde del círculo

        // Línea lateral izquierda
        vertices.push_back(center_x - line_offset);
        vertices.push_back(center_y);
        vertices.push_back(center_x - line_offset - line_length);
        vertices.push_back(center_y);

        // Línea lateral derecha
        vertices.push_back(center_x + line_offset);
        vertices.push_back(center_y);
        vertices.push_back(center_x + line_offset + line_length);
        vertices.push_back(center_y);
    }

    void PitchLadder::generatePitchLineVertices(std::vector<float> &vertices, float pitch_angle, float camera_pitch)
    {
        // Las líneas SE MUEVEN con el pitch de la cámara (pasan por la pantalla)
        // Calcular posición vertical basada en la DIFERENCIA entre el ángulo de la línea y el pitch actual
        float pitch_diff = pitch_angle - camera_pitch;
        float line_y = pitch_diff * 0.01f; // 1% por grado de diferencia

        // Solo mostrar líneas que estén dentro del rango visible
        if (std::abs(line_y) > 0.8f)
            return; // No mostrar líneas fuera de pantalla

        // Determinar el ancho de la línea según si es la línea central (0°) o no
        float line_width = (pitch_angle == 0.0f) ? 0.15f : 0.1f; // La línea de 0° es más larga
        float gap = 0.03f;                                       // Separación en el centro de la línea

        // Línea horizontal CORTADA AL MEDIO con separación
        // Parte izquierda (desde -line_width hasta -gap)
        vertices.push_back(-line_width);
        vertices.push_back(line_y);
        vertices.push_back(-gap);
        vertices.push_back(line_y);

        // Parte derecha (desde gap hasta line_width)
        vertices.push_back(gap);
        vertices.push_back(line_y);
        vertices.push_back(line_width);
        vertices.push_back(line_y);

        // Si no es la línea central, agregar marcadores en los extremos
        if (pitch_angle != 0.0f)
        {
            float marker_size = 0.02f;

            // Marcador izquierdo
            vertices.push_back(-line_width);
            vertices.push_back(line_y);
            vertices.push_back(-line_width);
            vertices.push_back(line_y + (pitch_angle > 0 ? marker_size : -marker_size));

            // Marcador derecho
            vertices.push_back(line_width);
            vertices.push_back(line_y);
            vertices.push_back(line_width);
            vertices.push_back(line_y + (pitch_angle > 0 ? marker_size : -marker_size));
        }
    }

    void PitchLadder::update(const hud::FlightData &data)
    {
        pitch_angle_deg_ = data.pitch; // grados
        updateModelMatrix();
    }

    void PitchLadder::render()
    {
        auto *shader = shader_;
        if (!shader || !shader->isCompiled())
            return;

        // Guardar estado GL
        GLboolean depthWasEnabled = glIsEnabled(GL_DEPTH_TEST);

        // Preparar el renderizado
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glLineWidth(2.0f);

        shader->use();

        // Configurar matriz de proyección ortográfica
        glm::mat4 projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f);
        shader->setMat4("projection", projection);

        // Color verde para el HUD
        shader->setVec3("color", glm::vec3(0.0f, 1.0f, 0.0f));
        shader->setFloat("alpha", 0.8f);

        std::vector<float> vertices;

        // 1. Generar la mira central (cruz) - FIJA en el centro de la pantalla
        generateCrosshairVertices(vertices, 0.0f, 0.0f);

        // 2. Generar líneas dinámicamente según el pitch actual (similar a bank_angle)
        // Calcular qué línea está más cerca del pitch actual
        int center_line_index = (int)round(pitch_angle_deg_ / PITCH_STEP);

        // Solo mostrar 5 líneas: 2 arriba, 1 central, 2 abajo del pitch actual
        // Esto hace que aparezcan/desaparezcan dinámicamente
        for (int i = center_line_index - 2; i <= center_line_index + 2; i++)
        {
            float pitch_line_angle = i * PITCH_STEP; // Múltiplos de 10°

            // Solo generar si está dentro del rango visual razonable
            if (pitch_line_angle >= -90.0f && pitch_line_angle <= 90.0f)
            {
                generatePitchLineVertices(vertices, pitch_line_angle, pitch_angle_deg_);
            }
        }

        // Cargar vertices al buffer
        glBindVertexArray(vao_);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);

        // Renderizar todas las líneas
        glDrawArrays(GL_LINES, 0, vertices.size() / 2);

        glBindVertexArray(0);

        // Restaurar estado GL
        if (depthWasEnabled)
            glEnable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);
        glLineWidth(1.0f);
    }

    void PitchLadder::updateModelMatrix()
    {
        // HUD 2D, matriz identidad
        model_matrix_ = glm::mat4(1.0f);
    }

} // namespace UI
