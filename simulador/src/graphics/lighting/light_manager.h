#pragma once

#include "light.h"
#include "../shaders/shader_manager.h"
#include <vector>
#include <memory>

namespace Graphics {
namespace Lighting {

    /**
     * @brief Gestor centralizado de luces en la escena
     */
    class LightManager {
    private:
        std::vector<std::unique_ptr<DirectionalLight>> directional_lights_;
        std::vector<std::unique_ptr<PointLight>> point_lights_;
        
        // Luz direccional principal (sol)
        DirectionalLight* main_light_;

    public:
        LightManager() : main_light_(nullptr) {}

        /**
         * @brief Añade una luz direccional
         */
        DirectionalLight* addDirectionalLight(DirectionalLight light) {
            auto ptr = std::make_unique<DirectionalLight>(std::move(light));
            DirectionalLight* raw_ptr = ptr.get();
            directional_lights_.push_back(std::move(ptr));
            
            // Si es la primera, hacerla principal
            if (!main_light_) {
                main_light_ = raw_ptr;
            }
            
            return raw_ptr;
        }

        /**
         * @brief Añade una luz puntual
         */
        PointLight* addPointLight(PointLight light) {
            auto ptr = std::make_unique<PointLight>(std::move(light));
            PointLight* raw_ptr = ptr.get();
            point_lights_.push_back(std::move(ptr));
            return raw_ptr;
        }

        /**
         * @brief Establece la luz direccional principal
         */
        void setMainLight(DirectionalLight* light) {
            main_light_ = light;
        }

        /**
         * @brief Obtiene la luz direccional principal
         */
        DirectionalLight* getMainLight() const {
            return main_light_;
        }

        /**
         * @brief Aplica las luces a un shader
         */
        void applyToShader(Shaders::Shader* shader) const {
            if (!shader) return;

            shader->use();

            // Aplicar luz direccional principal
            if (main_light_ && main_light_->isEnabled()) {
                shader->setVec3("dirLight.direction", main_light_->getDirection());
                shader->setVec3("dirLight.ambient", main_light_->getAmbient());
                shader->setVec3("dirLight.diffuse", main_light_->getDiffuse());
                shader->setVec3("dirLight.specular", main_light_->getSpecular());
                shader->setBool("dirLight.enabled", true);
            } else {
                shader->setBool("dirLight.enabled", false);
            }

            // Aplicar luces puntuales (hasta un máximo)
            const int MAX_POINT_LIGHTS = 4;
            int num_active_point_lights = 0;
            
            for (size_t i = 0; i < point_lights_.size() && i < MAX_POINT_LIGHTS; ++i) {
                const auto& light = point_lights_[i];
                if (!light->isEnabled()) continue;

                std::string base = "pointLights[" + std::to_string(num_active_point_lights) + "]";
                shader->setVec3(base + ".position", light->getPosition());
                shader->setVec3(base + ".ambient", light->getAmbient());
                shader->setVec3(base + ".diffuse", light->getDiffuse());
                shader->setVec3(base + ".specular", light->getSpecular());
                shader->setFloat(base + ".constant", light->getConstant());
                shader->setFloat(base + ".linear", light->getLinear());
                shader->setFloat(base + ".quadratic", light->getQuadratic());
                shader->setBool(base + ".enabled", true);
                
                num_active_point_lights++;
            }

            shader->setInt("numPointLights", num_active_point_lights);
        }

        /**
         * @brief Limpia todas las luces
         */
        void clear() {
            directional_lights_.clear();
            point_lights_.clear();
            main_light_ = nullptr;
        }

        /**
         * @brief Obtiene el número de luces direccionales
         */
        size_t getDirectionalLightCount() const {
            return directional_lights_.size();
        }

        /**
         * @brief Obtiene el número de luces puntuales
         */
        size_t getPointLightCount() const {
            return point_lights_.size();
        }
    };

} // namespace Lighting
} // namespace Graphics
