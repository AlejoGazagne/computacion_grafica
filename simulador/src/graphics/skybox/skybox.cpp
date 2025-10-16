#include "skybox.h"
#include "../shaders/shader_manager.h"
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
            : VAO_(0), VBO_(0), texture_id_(0), initialized_(false), shader_name_("skybox") {
        }

        Skybox::~Skybox() {
            cleanup();
        }

        Skybox::Skybox(Skybox&& other) noexcept 
            : VAO_(other.VAO_), VBO_(other.VBO_), texture_id_(other.texture_id_), 
              initialized_(other.initialized_), shader_name_(std::move(other.shader_name_)) {
            other.VAO_ = 0;
            other.VBO_ = 0;
            other.texture_id_ = 0;
            other.initialized_ = false;
        }

        Skybox& Skybox::operator=(Skybox&& other) noexcept {
            if (this != &other) {
                cleanup();
                
                VAO_ = other.VAO_;
                VBO_ = other.VBO_;
                texture_id_ = other.texture_id_;
                initialized_ = other.initialized_;
                shader_name_ = std::move(other.shader_name_);
                
                other.VAO_ = 0;
                other.VBO_ = 0;
                other.texture_id_ = 0;
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
            
            // Cargar shader usando ShaderManager
            auto& shader_manager = Shaders::ShaderManager::getInstance();
            if (!shader_manager.loadShader(shader_name_, 
                                          "shaders/vertex_skybox.glsl", 
                                          "shaders/fragment_skybox.glsl")) {
                std::cerr << "Failed to load skybox shader!" << std::endl;
                return false;
            }

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

        void Skybox::render(const glm::mat4& view, const glm::mat4& projection, bool fog_enabled) {
            if (!initialized_) return;

            // Obtener shader del ShaderManager
            auto& shader_manager = Shaders::ShaderManager::getInstance();
            Shaders::Shader* shader = shader_manager.getShader(shader_name_);
            if (!shader) {
                std::cerr << "Skybox shader not found!" << std::endl;
                return;
            }

            // Cambiar depth function para que el skybox se dibuje en el fondo
            glDepthFunc(GL_LEQUAL);
            
            shader->use();
            
            // Remover translación de la matriz view para el skybox
            glm::mat4 skybox_view = glm::mat4(glm::mat3(view));
            
            // Set uniforms
            shader->setMat4("view", skybox_view);
            shader->setMat4("projection", projection);
            
            // Configurar niebla para el skybox
            shader->setBool("fogEnabled", fog_enabled);
            shader->setFloat("fogDensity", 0.05f);
            shader->setVec3("fogColor", glm::vec3(0.7f, 0.8f, 0.9f));
            
            // Bind skybox texture
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, texture_id_);
            shader->setInt("skybox", 0);
            
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