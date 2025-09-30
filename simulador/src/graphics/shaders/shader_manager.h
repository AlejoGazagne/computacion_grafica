#ifndef SHADER_MANAGER_H
#define SHADER_MANAGER_H

extern "C" {
    #include <glad/glad.h>
}

#include <string>
#include <unordered_map>
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Graphics {
    namespace Shaders {

        class Shader {
        private:
            GLuint program_id_;
            std::string name_;
            bool compiled_;
            
            // Métodos auxiliares privados
            std::string loadShaderSource(const std::string& filepath);
            GLuint compileShader(const std::string& source, GLenum type);
            bool linkProgram(GLuint vertex_shader, GLuint fragment_shader, GLuint geometry_shader = 0);
            void checkCompileErrors(GLuint shader, const std::string& type);

        public:
            Shader();
            Shader(const std::string& name);
            ~Shader();

            // No permitir copia
            Shader(const Shader&) = delete;
            Shader& operator=(const Shader&) = delete;

            // Permitir movimiento
            Shader(Shader&& other) noexcept;
            Shader& operator=(Shader&& other) noexcept;

            bool loadFromFiles(const std::string& vertex_path, 
                             const std::string& fragment_path,
                             const std::string& geometry_path = "");
            
            bool loadFromSource(const std::string& vertex_source,
                              const std::string& fragment_source,
                              const std::string& geometry_source = "");

            void use() const;
            void unuse() const;
            
            GLuint getProgramId() const { return program_id_; }
            const std::string& getName() const { return name_; }
            bool isCompiled() const { return compiled_; }

            // Métodos para establecer uniformes
            void setBool(const std::string& name, bool value) const;
            void setInt(const std::string& name, int value) const;
            void setFloat(const std::string& name, float value) const;
            void setVec2(const std::string& name, const glm::vec2& value) const;
            void setVec3(const std::string& name, const glm::vec3& value) const;
            void setVec4(const std::string& name, const glm::vec4& value) const;
            void setMat2(const std::string& name, const glm::mat2& value) const;
            void setMat3(const std::string& name, const glm::mat3& value) const;
            void setMat4(const std::string& name, const glm::mat4& value) const;

            // Sobrecargas con punteros
            void setMat4(const std::string& name, const float* value) const;
            void setVec3(const std::string& name, float x, float y, float z) const;

        private:
            GLint getUniformLocation(const std::string& name) const;
        };

        class ShaderManager {
        private:
            std::unordered_map<std::string, std::unique_ptr<Shader>> shaders_;
            static std::unique_ptr<ShaderManager> instance_;

        public:
            ShaderManager() = default;
            ~ShaderManager() = default;

            // Singleton
            static ShaderManager& getInstance();
            
            // No permitir copia
            ShaderManager(const ShaderManager&) = delete;
            ShaderManager& operator=(const ShaderManager&) = delete;

            bool loadShader(const std::string& name,
                          const std::string& vertex_path,
                          const std::string& fragment_path,
                          const std::string& geometry_path = "");

            Shader* getShader(const std::string& name);
            void removeShader(const std::string& name);
            void clear();
            
            // Utilidades
            bool hasShader(const std::string& name) const;
            size_t getShaderCount() const;
        };

    } // namespace Shaders
} // namespace Graphics

#endif // SHADER_MANAGER_H