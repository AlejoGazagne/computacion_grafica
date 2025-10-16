#pragma once

#include <glm/glm.hpp>
#include <string>

namespace Graphics {
namespace Lighting {

    /**
     * @brief Tipos de luces soportadas
     */
    enum class LightType {
        DIRECTIONAL,  // Luz direccional (como el sol)
        POINT,        // Luz puntual
        SPOT          // Foco/spotlight
    };

    /**
     * @brief Clase base para todas las luces
     */
    class Light {
    protected:
        LightType type_;
        glm::vec3 color_;
        float intensity_;
        bool enabled_;
        std::string name_;

    public:
        Light(LightType type, const std::string& name = "light")
            : type_(type), color_(1.0f, 1.0f, 1.0f), 
              intensity_(1.0f), enabled_(true), name_(name) {}

        virtual ~Light() = default;

        // Getters
        LightType getType() const { return type_; }
        const glm::vec3& getColor() const { return color_; }
        float getIntensity() const { return intensity_; }
        bool isEnabled() const { return enabled_; }
        const std::string& getName() const { return name_; }

        // Setters
        void setColor(const glm::vec3& color) { color_ = color; }
        void setIntensity(float intensity) { intensity_ = intensity; }
        void setEnabled(bool enabled) { enabled_ = enabled; }
        void setName(const std::string& name) { name_ = name; }
    };

    /**
     * @brief Luz direccional (como el sol)
     * Ilumina toda la escena desde una dirección específica
     */
    class DirectionalLight : public Light {
    private:
        glm::vec3 direction_;
        glm::vec3 ambient_;
        glm::vec3 diffuse_;
        glm::vec3 specular_;

    public:
        DirectionalLight(const std::string& name = "sun")
            : Light(LightType::DIRECTIONAL, name),
              direction_(glm::normalize(glm::vec3(-0.2f, -1.0f, -0.3f))),
              ambient_(0.3f, 0.3f, 0.3f),
              diffuse_(0.8f, 0.8f, 0.8f),
              specular_(0.5f, 0.5f, 0.5f) {}

        // Getters
        const glm::vec3& getDirection() const { return direction_; }
        const glm::vec3& getAmbient() const { return ambient_; }
        const glm::vec3& getDiffuse() const { return diffuse_; }
        const glm::vec3& getSpecular() const { return specular_; }

        // Setters
        void setDirection(const glm::vec3& direction) { 
            direction_ = glm::normalize(direction); 
        }
        void setAmbient(const glm::vec3& ambient) { ambient_ = ambient; }
        void setDiffuse(const glm::vec3& diffuse) { diffuse_ = diffuse; }
        void setSpecular(const glm::vec3& specular) { specular_ = specular; }

        /**
         * @brief Configuración predeterminada para simular el sol
         */
        static DirectionalLight createSunlight() {
            DirectionalLight sun("sun");
            sun.setDirection(glm::vec3(-0.3f, -1.0f, -0.2f));  // Sol desde arriba
            sun.setAmbient(glm::vec3(0.5f, 0.5f, 0.5f));       // Luz ambiental uniforme
            sun.setDiffuse(glm::vec3(0.5f, 0.5f, 0.5f));       // Luz difusa uniforme
            sun.setSpecular(glm::vec3(0.0f, 0.0f, 0.0f));      // Sin especular
            sun.setIntensity(1.0f);
            return sun;
        }

        /**
         * @brief Configuración para simular la luna
         */
        static DirectionalLight createMoonlight() {
            DirectionalLight moon("moon");
            moon.setDirection(glm::vec3(0.3f, -1.0f, 0.2f));
            moon.setAmbient(glm::vec3(0.05f, 0.05f, 0.1f));     // Muy tenue
            moon.setDiffuse(glm::vec3(0.2f, 0.2f, 0.3f));       // Azulada
            moon.setSpecular(glm::vec3(0.3f, 0.3f, 0.4f));
            moon.setIntensity(0.3f);
            return moon;
        }
    };

    /**
     * @brief Luz puntual (como una bombilla)
     * Ilumina desde un punto específico con atenuación
     */
    class PointLight : public Light {
    private:
        glm::vec3 position_;
        glm::vec3 ambient_;
        glm::vec3 diffuse_;
        glm::vec3 specular_;
        
        // Parámetros de atenuación
        float constant_;
        float linear_;
        float quadratic_;

    public:
        PointLight(const glm::vec3& position = glm::vec3(0.0f), 
                   const std::string& name = "point_light")
            : Light(LightType::POINT, name),
              position_(position),
              ambient_(0.2f, 0.2f, 0.2f),
              diffuse_(0.8f, 0.8f, 0.8f),
              specular_(1.0f, 1.0f, 1.0f),
              constant_(1.0f),
              linear_(0.09f),
              quadratic_(0.032f) {}

        // Getters
        const glm::vec3& getPosition() const { return position_; }
        const glm::vec3& getAmbient() const { return ambient_; }
        const glm::vec3& getDiffuse() const { return diffuse_; }
        const glm::vec3& getSpecular() const { return specular_; }
        float getConstant() const { return constant_; }
        float getLinear() const { return linear_; }
        float getQuadratic() const { return quadratic_; }

        // Setters
        void setPosition(const glm::vec3& position) { position_ = position; }
        void setAmbient(const glm::vec3& ambient) { ambient_ = ambient; }
        void setDiffuse(const glm::vec3& diffuse) { diffuse_ = diffuse; }
        void setSpecular(const glm::vec3& specular) { specular_ = specular; }
        void setAttenuation(float constant, float linear, float quadratic) {
            constant_ = constant;
            linear_ = linear;
            quadratic_ = quadratic;
        }
    };

} // namespace Lighting
} // namespace Graphics
