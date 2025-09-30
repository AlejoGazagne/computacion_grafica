#include "shader_manager.h"
#include <fstream>
#include <sstream>
#include <iostream>

namespace Graphics {
    namespace Shaders {

        // Inicialización del singleton
        std::unique_ptr<ShaderManager> ShaderManager::instance_ = nullptr;

        // === Implementación de Shader ===

        Shader::Shader() : program_id_(0), name_(""), compiled_(false) {
        }

        Shader::Shader(const std::string& name) 
            : program_id_(0), name_(name), compiled_(false) {
        }

        Shader::~Shader() {
            if (program_id_ != 0) {
                glDeleteProgram(program_id_);
            }
        }

        Shader::Shader(Shader&& other) noexcept 
            : program_id_(other.program_id_), name_(std::move(other.name_)), compiled_(other.compiled_) {
            other.program_id_ = 0;
            other.compiled_ = false;
        }

        Shader& Shader::operator=(Shader&& other) noexcept {
            if (this != &other) {
                if (program_id_ != 0) {
                    glDeleteProgram(program_id_);
                }
                
                program_id_ = other.program_id_;
                name_ = std::move(other.name_);
                compiled_ = other.compiled_;
                
                other.program_id_ = 0;
                other.compiled_ = false;
            }
            return *this;
        }

        std::string Shader::loadShaderSource(const std::string& filepath) {
            std::ifstream file;
            std::stringstream stream;
            
            file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
            
            try {
                file.open(filepath);
                stream << file.rdbuf();
                file.close();
                return stream.str();
            } catch (std::ifstream::failure& e) {
                std::cerr << "ERROR: Failed to read shader file: " << filepath << std::endl;
                std::cerr << "Error: " << e.what() << std::endl;
                return "";
            }
        }

        GLuint Shader::compileShader(const std::string& source, GLenum type) {
            GLuint shader = glCreateShader(type);
            const char* source_cstr = source.c_str();
            
            glShaderSource(shader, 1, &source_cstr, nullptr);
            glCompileShader(shader);
            
            // Verificar errores de compilación
            std::string type_str;
            switch (type) {
                case GL_VERTEX_SHADER: type_str = "VERTEX"; break;
                case GL_FRAGMENT_SHADER: type_str = "FRAGMENT"; break;
                case GL_GEOMETRY_SHADER: type_str = "GEOMETRY"; break;
                default: type_str = "UNKNOWN"; break;
            }
            
            checkCompileErrors(shader, type_str);
            return shader;
        }

        bool Shader::linkProgram(GLuint vertex_shader, GLuint fragment_shader, GLuint geometry_shader) {
            program_id_ = glCreateProgram();
            
            glAttachShader(program_id_, vertex_shader);
            glAttachShader(program_id_, fragment_shader);
            
            if (geometry_shader != 0) {
                glAttachShader(program_id_, geometry_shader);
            }
            
            glLinkProgram(program_id_);
            
            // Verificar errores de enlazado
            checkCompileErrors(program_id_, "PROGRAM");
            
            // Limpiar shaders (ya no los necesitamos después del enlazado)
            glDeleteShader(vertex_shader);
            glDeleteShader(fragment_shader);
            if (geometry_shader != 0) {
                glDeleteShader(geometry_shader);
            }
            
            return compiled_;
        }

        void Shader::checkCompileErrors(GLuint shader, const std::string& type) {
            GLint success;
            GLchar info_log[1024];
            
            if (type != "PROGRAM") {
                glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
                if (!success) {
                    glGetShaderInfoLog(shader, 1024, nullptr, info_log);
                    std::cerr << "ERROR: Shader compilation failed (" << type << ")" << std::endl;
                    std::cerr << "Shader: " << name_ << std::endl;
                    std::cerr << "Info Log: " << info_log << std::endl;
                    compiled_ = false;
                    return;
                }
            } else {
                glGetProgramiv(shader, GL_LINK_STATUS, &success);
                if (!success) {
                    glGetProgramInfoLog(shader, 1024, nullptr, info_log);
                    std::cerr << "ERROR: Program linking failed" << std::endl;
                    std::cerr << "Program: " << name_ << std::endl;
                    std::cerr << "Info Log: " << info_log << std::endl;
                    compiled_ = false;
                    return;
                }
            }
            
            compiled_ = true;
        }

        bool Shader::loadFromFiles(const std::string& vertex_path, 
                                 const std::string& fragment_path,
                                 const std::string& geometry_path) {
            
            std::string vertex_source = loadShaderSource(vertex_path);
            std::string fragment_source = loadShaderSource(fragment_path);
            std::string geometry_source = "";
            
            if (!geometry_path.empty()) {
                geometry_source = loadShaderSource(geometry_path);
            }
            
            if (vertex_source.empty() || fragment_source.empty()) {
                return false;
            }
            
            return loadFromSource(vertex_source, fragment_source, geometry_source);
        }

        bool Shader::loadFromSource(const std::string& vertex_source,
                                  const std::string& fragment_source,
                                  const std::string& geometry_source) {
            
            compiled_ = false;
            
            // Compilar shaders
            GLuint vertex_shader = compileShader(vertex_source, GL_VERTEX_SHADER);
            if (!compiled_) return false;
            
            GLuint fragment_shader = compileShader(fragment_source, GL_FRAGMENT_SHADER);
            if (!compiled_) {
                glDeleteShader(vertex_shader);
                return false;
            }
            
            GLuint geometry_shader = 0;
            if (!geometry_source.empty()) {
                geometry_shader = compileShader(geometry_source, GL_GEOMETRY_SHADER);
                if (!compiled_) {
                    glDeleteShader(vertex_shader);
                    glDeleteShader(fragment_shader);
                    return false;
                }
            }
            
            // Enlazar programa
            return linkProgram(vertex_shader, fragment_shader, geometry_shader);
        }

        void Shader::use() const {
            if (compiled_ && program_id_ != 0) {
                glUseProgram(program_id_);
            }
        }

        void Shader::unuse() const {
            glUseProgram(0);
        }

        GLint Shader::getUniformLocation(const std::string& name) const {
            GLint location = glGetUniformLocation(program_id_, name.c_str());
            if (location == -1) {
                std::cerr << "WARNING: Uniform '" << name << "' not found in shader '" << name_ << "'" << std::endl;
            }
            return location;
        }

        // Implementación de métodos para uniformes
        void Shader::setBool(const std::string& name, bool value) const {
            glUniform1i(getUniformLocation(name), static_cast<int>(value));
        }

        void Shader::setInt(const std::string& name, int value) const {
            glUniform1i(getUniformLocation(name), value);
        }

        void Shader::setFloat(const std::string& name, float value) const {
            glUniform1f(getUniformLocation(name), value);
        }

        void Shader::setVec2(const std::string& name, const glm::vec2& value) const {
            glUniform2fv(getUniformLocation(name), 1, &value[0]);
        }

        void Shader::setVec3(const std::string& name, const glm::vec3& value) const {
            glUniform3fv(getUniformLocation(name), 1, &value[0]);
        }

        void Shader::setVec3(const std::string& name, float x, float y, float z) const {
            glUniform3f(getUniformLocation(name), x, y, z);
        }

        void Shader::setVec4(const std::string& name, const glm::vec4& value) const {
            glUniform4fv(getUniformLocation(name), 1, &value[0]);
        }

        void Shader::setMat2(const std::string& name, const glm::mat2& value) const {
            glUniformMatrix2fv(getUniformLocation(name), 1, GL_FALSE, &value[0][0]);
        }

        void Shader::setMat3(const std::string& name, const glm::mat3& value) const {
            glUniformMatrix3fv(getUniformLocation(name), 1, GL_FALSE, &value[0][0]);
        }

        void Shader::setMat4(const std::string& name, const glm::mat4& value) const {
            glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, &value[0][0]);
        }

        void Shader::setMat4(const std::string& name, const float* value) const {
            glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, value);
        }

        // === Implementación de ShaderManager ===

        ShaderManager& ShaderManager::getInstance() {
            if (!instance_) {
                instance_ = std::make_unique<ShaderManager>();
            }
            return *instance_;
        }

        bool ShaderManager::loadShader(const std::string& name,
                                     const std::string& vertex_path,
                                     const std::string& fragment_path,
                                     const std::string& geometry_path) {
            
            auto shader = std::make_unique<Shader>(name);
            
            if (!shader->loadFromFiles(vertex_path, fragment_path, geometry_path)) {
                std::cerr << "Failed to load shader: " << name << std::endl;
                return false;
            }
            
            shaders_[name] = std::move(shader);
            std::cout << "Shader loaded successfully: " << name << std::endl;
            return true;
        }

        Shader* ShaderManager::getShader(const std::string& name) {
            auto it = shaders_.find(name);
            if (it != shaders_.end()) {
                return it->second.get();
            }
            
            std::cerr << "Shader not found: " << name << std::endl;
            return nullptr;
        }

        void ShaderManager::removeShader(const std::string& name) {
            shaders_.erase(name);
        }

        void ShaderManager::clear() {
            shaders_.clear();
        }

        bool ShaderManager::hasShader(const std::string& name) const {
            return shaders_.find(name) != shaders_.end();
        }

        size_t ShaderManager::getShaderCount() const {
            return shaders_.size();
        }

    } // namespace Shaders
} // namespace Graphics