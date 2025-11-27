#pragma once

#include "light.h"
#include "../shaders/shader_manager.h"
#include <memory>

namespace Graphics
{
    namespace Lighting
    {

        /**
         * @brief Gestor simplificado de luz solar para el simulador
         */
        class LightManager
        {
        private:
            std::unique_ptr<DirectionalLight> sun_;

        public:
            LightManager() : sun_(nullptr) {}

            /**
             * @brief Establece la luz solar
             */
            void setSunlight(DirectionalLight light)
            {
                sun_ = std::make_unique<DirectionalLight>(std::move(light));
            }

            /**
             * @brief Obtiene la luz solar
             */
            DirectionalLight *getSunlight() const
            {
                return sun_.get();
            }

            /**
             * @brief Aplica la luz solar a un shader
             */
            void applyToShader(Shaders::Shader *shader) const
            {
                if (!shader || !sun_)
                    return;

                shader->use();

                if (sun_->isEnabled())
                {
                    shader->setVec3("dirLight.direction", sun_->getDirection());
                    shader->setVec3("dirLight.ambient", sun_->getAmbient());
                    shader->setVec3("dirLight.diffuse", sun_->getDiffuse());
                    shader->setVec3("dirLight.specular", sun_->getSpecular());
                    shader->setBool("dirLight.enabled", true);
                }
                else
                {
                    shader->setBool("dirLight.enabled", false);
                }
            }
        };

    } // namespace Lighting
} // namespace Graphics
