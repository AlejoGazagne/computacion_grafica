#include "skybox.h"
#include "../shaders/shader_manager.h"
#include "../textures/texture_manager.h"
#include <iostream>
#include <filesystem>
#include <glm/gtc/matrix_transform.hpp>

namespace Graphics
{
    namespace Skybox
    {

        // Vertices del cubo para el skybox (posiciones únicamente)
        const float Skybox::skybox_vertices_[] = {
            // Posiciones
            -1.0f, 1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, 1.0f, -1.0f,
            -1.0f, 1.0f, -1.0f,

            -1.0f, -1.0f, 1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f, 1.0f, -1.0f,
            -1.0f, 1.0f, -1.0f,
            -1.0f, 1.0f, 1.0f,
            -1.0f, -1.0f, 1.0f,

            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,

            -1.0f, -1.0f, 1.0f,
            -1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            1.0f, -1.0f, 1.0f,
            -1.0f, -1.0f, 1.0f,

            -1.0f, 1.0f, -1.0f,
            1.0f, 1.0f, -1.0f,
            1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            -1.0f, 1.0f, 1.0f,
            -1.0f, 1.0f, -1.0f,

            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f, 1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f, 1.0f,
            1.0f, -1.0f, 1.0f};

        Skybox::Skybox()
            : VAO_(0), VBO_(0), texture_name_(""), initialized_(false), shader_name_("skybox")
        {
        }

        Skybox::~Skybox()
        {
            cleanup();
        }

        Skybox::Skybox(Skybox &&other) noexcept
            : VAO_(other.VAO_), VBO_(other.VBO_), texture_name_(std::move(other.texture_name_)),
              initialized_(other.initialized_), shader_name_(std::move(other.shader_name_))
        {
            other.VAO_ = 0;
            other.VBO_ = 0;
            other.texture_name_ = "";
            other.initialized_ = false;
        }

        Skybox &Skybox::operator=(Skybox &&other) noexcept
        {
            if (this != &other)
            {
                cleanup();

                VAO_ = other.VAO_;
                VBO_ = other.VBO_;
                texture_name_ = std::move(other.texture_name_);
                initialized_ = other.initialized_;
                shader_name_ = std::move(other.shader_name_);

                other.VAO_ = 0;
                other.VBO_ = 0;
                other.texture_name_ = "";
                other.initialized_ = false;
            }
            return *this;
        }

        bool Skybox::initialize(const SkyboxConfig &config)
        {
            if (initialized_)
            {
                std::cerr << "Skybox already initialized!" << std::endl;
                return false;
            }

            // Validar texturas
            if (!Utils::validateSkyboxTextures(config.faces_paths))
            {
                std::cerr << "Invalid skybox texture paths!" << std::endl;
                return false;
            }

            // Cargar cubemap usando TextureManager
            auto &texture_manager = Textures::TextureManager::getInstance();

            // Preparar FaceTextures para el TextureManager
            std::vector<Textures::FaceTexture> face_textures;
            const std::vector<Textures::CubeFace> faces = {
                Textures::CubeFace::POSITIVE_X, Textures::CubeFace::NEGATIVE_X,
                Textures::CubeFace::POSITIVE_Y, Textures::CubeFace::NEGATIVE_Y,
                Textures::CubeFace::POSITIVE_Z, Textures::CubeFace::NEGATIVE_Z};

            for (size_t i = 0; i < config.faces_paths.size(); ++i)
            {
                Textures::FaceTexture face_tex;
                face_tex.filepath = config.faces_paths[i];
                face_tex.face = faces[i];
                face_tex.flip_vertically = config.flip_y;
                face_textures.push_back(face_tex);
            }

            if (!texture_manager.loadCubemap(config.texture_name, face_textures))
            {
                std::cerr << "Failed to load skybox cubemap via TextureManager!" << std::endl;
                return false;
            }

            texture_name_ = config.texture_name;

            // Setup mesh
            setupMesh();

            // Cargar shader usando ShaderManager
            auto &shader_manager = Shaders::ShaderManager::getInstance();
            if (!shader_manager.loadShader(shader_name_,
                                           "shaders/vertex_skybox.glsl",
                                           "shaders/fragment_skybox.glsl"))
            {
                std::cerr << "Failed to load skybox shader!" << std::endl;
                return false;
            }

            initialized_ = true;
            std::cout << "Skybox initialized successfully using TextureManager" << std::endl;
            return true;
        }

        void Skybox::setupMesh()
        {
            glGenVertexArrays(1, &VAO_);
            glGenBuffers(1, &VBO_);

            glBindVertexArray(VAO_);
            glBindBuffer(GL_ARRAY_BUFFER, VBO_);
            glBufferData(GL_ARRAY_BUFFER, sizeof(skybox_vertices_), &skybox_vertices_, GL_STATIC_DRAW);

            // Posiciones (location = 0)
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
            glEnableVertexAttribArray(0);

            glBindVertexArray(0);
        }

        void Skybox::render(const glm::mat4 &view, const glm::mat4 &projection, bool fog_enabled)
        {
            if (!initialized_)
                return;

            // Obtener shader del ShaderManager
            auto &shader_manager = Shaders::ShaderManager::getInstance();
            Shaders::Shader *shader = shader_manager.getShader(shader_name_);
            if (!shader)
            {
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

            // Configurar niebla para el skybox (alineada con el terreno)
            shader->setBool("fogEnabled", fog_enabled);
            shader->setFloat("fogDensity", 0.0001f);
            shader->setVec3("fogColor", glm::vec3(0.85f, 0.90f, 0.95f));

            // Obtener y bindear la textura del cubemap desde TextureManager
            auto &texture_manager = Textures::TextureManager::getInstance();
            Textures::Texture *cubemap_texture = texture_manager.getTexture(texture_name_);
            if (cubemap_texture)
            {
                cubemap_texture->bind(0);
            }
            else
            {
                std::cerr << "Skybox texture not found in TextureManager: " << texture_name_ << std::endl;
                glDepthFunc(GL_LESS);
                return;
            }
            shader->setFloat("skyFogScale", 8000.0f); // menor impacto global
            shader->setFloat("skyFogMax", 0.8f);      // tope más bajo
            shader->setFloat("skyFogExponent", 2.5f); // banda más fina en el horizonte
            shader->setInt("skybox", 0);              // Texture unit 0

            // Render skybox cube
            glBindVertexArray(VAO_);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            glBindVertexArray(0);

            // Restaurar depth function
            glDepthFunc(GL_LESS);
        }

        void Skybox::cleanup()
        {
            if (VAO_ != 0)
            {
                glDeleteVertexArrays(1, &VAO_);
                VAO_ = 0;
            }
            if (VBO_ != 0)
            {
                glDeleteBuffers(1, &VBO_);
                VBO_ = 0;
            }
            
            texture_name_ = "";
            initialized_ = false;
        }

        // Utility functions
        namespace Utils
        {
            bool validateSkyboxTextures(const std::vector<std::string> &faces_paths)
            {
                if (faces_paths.size() != 6)
                {
                    std::cerr << "Skybox requires exactly 6 texture faces, got " << faces_paths.size() << std::endl;
                    return false;
                }

                for (const auto &path : faces_paths)
                {
                    if (!std::filesystem::exists(path))
                    {
                        std::cerr << "Skybox texture not found: " << path << std::endl;
                        return false;
                    }
                }

                return true;
            }

            std::vector<std::string> getSkyboxFacesFromDirectory(const std::string &directory)
            {
                return {
                    directory + "/right.png",
                    directory + "/left.png",
                    directory + "/top.png",
                    directory + "/bottom.png",
                    directory + "/front.png",
                    directory + "/back.png"};
            }
        }

    } // namespace Skybox
} // namespace Graphics