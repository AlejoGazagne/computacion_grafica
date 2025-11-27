#pragma once

#include <glm/glm.hpp>

namespace Graphics
{
    namespace Lighting
    {

        /**
         * @brief Luz direccional (como el sol)
         * Ilumina toda la escena desde una dirección específica
         */
        class DirectionalLight
        {
        private:
            glm::vec3 direction_;
            glm::vec3 ambient_;
            glm::vec3 diffuse_;
            glm::vec3 specular_;
            bool enabled_;

        public:
            DirectionalLight()
                : direction_(glm::normalize(glm::vec3(-0.2f, -1.0f, -0.3f))),
                  ambient_(0.3f, 0.3f, 0.3f),
                  diffuse_(0.8f, 0.8f, 0.8f),
                  specular_(0.5f, 0.5f, 0.5f),
                  enabled_(true) {}

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
