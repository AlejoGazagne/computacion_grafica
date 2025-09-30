#include "skybox.h"
#include <stb_image.h>
#include <iostream>
#include <filesystem>
#include <glm/gtc/matrix_transform.hpp>

namespace Graphics {
    namespace Skybox {

        // Vertices del cubo para el skybox (posiciones únicamente)
        const float Skybox::skybox_vertices_[] = {
            // Posiciones          
            -1.0f,  1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

             1.0f, -1.0f, -1.0f,
             1.0f, -1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            -1.0f,  1.0f, -1.0f,
             1.0f,  1.0f, -1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
             1.0f, -1.0f,  1.0f
        };

        Skybox::Skybox() 
            : VAO_(0), VBO_(0), texture_id_(0), shader_program_(0), initialized_(false) {
        }

        Skybox::~Skybox() {
            cleanup();
        }

        Skybox::Skybox(Skybox&& other) noexcept 
            : VAO_(other.VAO_), VBO_(other.VBO_), texture_id_(other.texture_id_), 
              shader_program_(other.shader_program_), initialized_(other.initialized_) {
            other.VAO_ = 0;
            other.VBO_ = 0;
            other.texture_id_ = 0;
            other.shader_program_ = 0;
            other.initialized_ = false;
        }

        Skybox& Skybox::operator=(Skybox&& other) noexcept {
            if (this != &other) {
                cleanup();
                
                VAO_ = other.VAO_;
                VBO_ = other.VBO_;
                texture_id_ = other.texture_id_;
                shader_program_ = other.shader_program_;
                initialized_ = other.initialized_;
                
                other.VAO_ = 0;
                other.VBO_ = 0;
                other.texture_id_ = 0;
                other.shader_program_ = 0;
                other.initialized_ = false;
            }
            return *this;
        }

        bool Skybox::initialize(const SkyboxConfig& config) {
            if (initialized_) {
                std::cerr << "Skybox already initialized!" << std::endl;
                return false;
            }

            // Validar texturas
            if (!Utils::validateSkyboxTextures(config.faces_paths)) {
                std::cerr << "Invalid skybox texture paths!" << std::endl;
                return false;
            }

            // Cargar cubemap
            if (!loadCubemap(config.faces_paths, config.flip_y)) {
                std::cerr << "Failed to load skybox cubemap!" << std::endl;
                return false;
            }

            // Setup mesh
            setupMesh();
            
            // Setup shader
            setupShader();

            initialized_ = true;
            std::cout << "Skybox initialized successfully" << std::endl;
            return true;
        }

        bool Skybox::loadCubemap(const std::vector<std::string>& faces_paths, bool flip_y) {
            glGenTextures(1, &texture_id_);
            glBindTexture(GL_TEXTURE_CUBE_MAP, texture_id_);

            stbi_set_flip_vertically_on_load(flip_y);

            int width, height, channels;
            for (unsigned int i = 0; i < faces_paths.size(); i++) {
                unsigned char* data = stbi_load(faces_paths[i].c_str(), &width, &height, &channels, 0);
                if (data) {
                    GLenum format = (channels == 3) ? GL_RGB : GL_RGBA;
                    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 
                                 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
                    std::cout << "Loaded skybox face: " << faces_paths[i] 
                              << " (" << width << "x" << height << ", " << channels << " channels)" << std::endl;
                    stbi_image_free(data);
                } else {
                    std::cerr << "Failed to load skybox face: " << faces_paths[i] << std::endl;
                    stbi_image_free(data);
                    return false;
                }
            }

            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

            return true;
        }

        void Skybox::setupMesh() {
            glGenVertexArrays(1, &VAO_);
            glGenBuffers(1, &VBO_);

            glBindVertexArray(VAO_);
            glBindBuffer(GL_ARRAY_BUFFER, VBO_);
            glBufferData(GL_ARRAY_BUFFER, sizeof(skybox_vertices_), &skybox_vertices_, GL_STATIC_DRAW);

            // Posiciones (location = 0)
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);

            glBindVertexArray(0);
        }

        void Skybox::setupShader() {
            const char* vertex_shader_source = R"(
                #version 330 core
                layout (location = 0) in vec3 aPos;

                out vec3 TexCoords;

                uniform mat4 projection;
                uniform mat4 view;

                void main() {
                    TexCoords = aPos;
                    vec4 pos = projection * view * vec4(aPos, 1.0);
                    gl_Position = pos.xyww;
                }
            )";

            const char* fragment_shader_source = R"(
                #version 330 core
                out vec4 FragColor;

                in vec3 TexCoords;

                uniform samplerCube skybox;

                void main() {    
                    FragColor = texture(skybox, TexCoords);
                }
            )";

            // Compilar vertex shader
            GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
            glShaderSource(vertex_shader, 1, &vertex_shader_source, NULL);
            glCompileShader(vertex_shader);

            // Verificar compilación
            int success;
            char infoLog[512];
            glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
            if (!success) {
                glGetShaderInfoLog(vertex_shader, 512, NULL, infoLog);
                std::cerr << "Skybox vertex shader compilation failed: " << infoLog << std::endl;
            }

            // Compilar fragment shader
            GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
            glShaderSource(fragment_shader, 1, &fragment_shader_source, NULL);
            glCompileShader(fragment_shader);

            glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
            if (!success) {
                glGetShaderInfoLog(fragment_shader, 512, NULL, infoLog);
                std::cerr << "Skybox fragment shader compilation failed: " << infoLog << std::endl;
            }

            // Crear programa
            shader_program_ = glCreateProgram();
            glAttachShader(shader_program_, vertex_shader);
            glAttachShader(shader_program_, fragment_shader);
            glLinkProgram(shader_program_);

            glGetProgramiv(shader_program_, GL_LINK_STATUS, &success);
            if (!success) {
                glGetProgramInfoLog(shader_program_, 512, NULL, infoLog);
                std::cerr << "Skybox shader program linking failed: " << infoLog << std::endl;
            }

            glDeleteShader(vertex_shader);
            glDeleteShader(fragment_shader);
        }

        void Skybox::render(const glm::mat4& view, const glm::mat4& projection) {
            if (!initialized_) return;

            // Cambiar depth function para que el skybox se dibuje en el fondo
            glDepthFunc(GL_LEQUAL);
            
            glUseProgram(shader_program_);
            
            // Remover translación de la matriz view para el skybox
            glm::mat4 skybox_view = glm::mat4(glm::mat3(view));
            
            // Set uniforms
            glUniformMatrix4fv(glGetUniformLocation(shader_program_, "view"), 1, GL_FALSE, &skybox_view[0][0]);
            glUniformMatrix4fv(glGetUniformLocation(shader_program_, "projection"), 1, GL_FALSE, &projection[0][0]);
            
            // Bind skybox texture
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, texture_id_);
            glUniform1i(glGetUniformLocation(shader_program_, "skybox"), 0);
            
            // Render skybox cube
            glBindVertexArray(VAO_);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            glBindVertexArray(0);
            
            // Restaurar depth function
            glDepthFunc(GL_LESS);
        }

        void Skybox::cleanup() {
            if (VAO_) {
                glDeleteVertexArrays(1, &VAO_);
                VAO_ = 0;
            }
            if (VBO_) {
                glDeleteBuffers(1, &VBO_);
                VBO_ = 0;
            }
            if (texture_id_) {
                glDeleteTextures(1, &texture_id_);
                texture_id_ = 0;
            }
            if (shader_program_) {
                glDeleteProgram(shader_program_);
                shader_program_ = 0;
            }
            initialized_ = false;
        }

        // Utility functions
        namespace Utils {
            bool validateSkyboxTextures(const std::vector<std::string>& faces_paths) {
                if (faces_paths.size() != 6) {
                    std::cerr << "Skybox requires exactly 6 texture faces, got " << faces_paths.size() << std::endl;
                    return false;
                }

                for (const auto& path : faces_paths) {
                    if (!std::filesystem::exists(path)) {
                        std::cerr << "Skybox texture not found: " << path << std::endl;
                        return false;
                    }
                }

                return true;
            }

            std::vector<std::string> getSkyboxFacesFromDirectory(const std::string& directory) {
                return {
                    directory + "/right.png",
                    directory + "/left.png", 
                    directory + "/top.png",
                    directory + "/bottom.png",
                    directory + "/front.png",
                    directory + "/back.png"
                };
            }
        }

    } // namespace Skybox
} // namespace Graphics