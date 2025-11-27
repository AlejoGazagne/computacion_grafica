#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Graphics
{
    namespace Lighting
    {

        /**
         * @brief Luz direccional (como el sol)
         * Ilumina toda la escena desde una dirección específica
         * Incluye cálculo de matriz lightSpace para shadow mapping
         */
        class DirectionalLight
        {
        private:
            glm::vec3 direction_;
            glm::vec3 ambient_;
            glm::vec3 diffuse_;
            glm::vec3 specular_;
            bool enabled_;

            // Shadow mapping - Parámetros del frustum ortográfico de la luz
            float shadow_frustum_size_; // Tamaño del frustum ortográfico
            float shadow_near_plane_;   // Plano cercano
            float shadow_far_plane_;    // Plano lejano

        public:
            DirectionalLight()
                : direction_(glm::normalize(glm::vec3(-0.2f, -1.0f, -0.3f))),
                  ambient_(0.3f, 0.3f, 0.3f),
                  diffuse_(0.8f, 0.8f, 0.8f),
                  specular_(0.5f, 0.5f, 0.5f),
                  enabled_(true),
                  shadow_frustum_size_(100.0f),
                  shadow_near_plane_(1.0f),
                  shadow_far_plane_(300.0f) {}

            // Getters
            const glm::vec3 &getDirection() const { return direction_; }
            const glm::vec3 &getAmbient() const { return ambient_; }
            const glm::vec3 &getDiffuse() const { return diffuse_; }
            const glm::vec3 &getSpecular() const { return specular_; }
            bool isEnabled() const { return enabled_; }

            // Setters
            void setDirection(const glm::vec3 &direction)
            {
                direction_ = glm::normalize(direction);
            }
            void setAmbient(const glm::vec3 &ambient) { ambient_ = ambient; }
            void setDiffuse(const glm::vec3 &diffuse) { diffuse_ = diffuse; }
            void setSpecular(const glm::vec3 &specular) { specular_ = specular; }
            void setEnabled(bool enabled) { enabled_ = enabled; }

            // Shadow mapping setters
            void setShadowFrustumSize(float size) { shadow_frustum_size_ = size; }
            void setShadowPlanes(float near_plane, float far_plane)
            {
                shadow_near_plane_ = near_plane;
                shadow_far_plane_ = far_plane;
            }

            /**
             * @brief Calcula la matriz lightSpaceMatrix para shadow mapping
             *
             * Basado en la teoría de transformaciones de OpenGL:
             * lightSpaceMatrix = Projection * View
             *
             * - View: lookAt desde la posición de la luz mirando en su dirección
             * - Projection: Ortográfica (luces direccionales son paralelas)
             *
             * @param target_position Posición del objeto a seguir (ej: avión)
             * @return Matriz que transforma de world space a light clip space
             */
            glm::mat4 getLightSpaceMatrix(const glm::vec3 &target_position) const
            {
                // Posición de la luz: alejada en dirección opuesta a la luz
                // Para luz direccional, la posición no importa (rayos paralelos)
                // pero necesitamos un punto para lookAt
                glm::vec3 light_pos = target_position - direction_ * (shadow_far_plane_ * 0.5f);

                // Matriz de vista: desde la luz mirando hacia la escena
                glm::mat4 light_view = glm::lookAt(
                    light_pos,                  // Posición de la luz
                    target_position,            // Mirando hacia el objetivo
                    glm::vec3(0.0f, 1.0f, 0.0f) // Up vector
                );

                // Proyección ortográfica para luz direccional
                // Crea un volumen de visión rectangular (frustum box)
                float half_size = shadow_frustum_size_ * 0.5f;
                glm::mat4 light_projection = glm::ortho(
                    -half_size, half_size, // left, right
                    -half_size, half_size, // bottom, top
                    shadow_near_plane_,    // near
                    shadow_far_plane_      // far
                );

                // lightSpaceMatrix = Projection * View
                return light_projection * light_view;
            }

            /**
             * @brief Configuración predeterminada para simular el sol
             */
            static DirectionalLight createSunlight()
            {
                DirectionalLight sun;
                sun.setDirection(glm::vec3(-0.3f, -1.0f, -0.2f)); // Sol desde arriba
                sun.setAmbient(glm::vec3(0.5f, 0.5f, 0.5f));      // Luz ambiental uniforme
                sun.setDiffuse(glm::vec3(0.5f, 0.5f, 0.5f));      // Luz difusa uniforme
                sun.setSpecular(glm::vec3(0.0f, 0.0f, 0.0f));     // Sin especular
                sun.setEnabled(true);
                return sun;
            }
        };

    } // namespace Lighting
} // namespace Graphics
