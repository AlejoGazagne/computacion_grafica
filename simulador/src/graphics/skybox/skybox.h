#pragma once

#include <glad/glad.h>
#include <string>
#include <vector>
#include <glm/glm.hpp>

namespace Graphics {
    namespace Skybox {

        struct SkyboxConfig {
            std::vector<std::string> faces_paths;
            bool flip_y = false;
            
            static SkyboxConfig createDefault() {
                return {
                    {
                        "textures/skybox/right.png",   // +X
                        "textures/skybox/left.png",    // -X
                        "textures/skybox/top.png",     // +Y
                        "textures/skybox/bottom.png",  // -Y
                        "textures/skybox/front.png",   // +Z
                        "textures/skybox/back.png"     // -Z
                    },
                    false
                };
            }
        };

        class Skybox {
        private:
            GLuint VAO_, VBO_;
            GLuint texture_id_;
            bool initialized_;
            std::string shader_name_;

            // Vertices del cubo para el skybox (solo posiciones)
            static const float skybox_vertices_[];

            bool loadCubemap(const std::vector<std::string>& faces_paths, bool flip_y = false);
            void setupMesh();

        public:
            Skybox();
            ~Skybox();

            // No copiable
            Skybox(const Skybox&) = delete;
            Skybox& operator=(const Skybox&) = delete;

            // Movible
            Skybox(Skybox&& other) noexcept;
            Skybox& operator=(Skybox&& other) noexcept;

            bool initialize(const SkyboxConfig& config = SkyboxConfig::createDefault());
            void render(const glm::mat4& view, const glm::mat4& projection, bool fog_enabled = true);
            void cleanup();

            bool isInitialized() const { return initialized_; }
            GLuint getTextureId() const { return texture_id_; }
        };

        // Utility functions
        namespace Utils {
            bool validateSkyboxTextures(const std::vector<std::string>& faces_paths);
            std::vector<std::string> getSkyboxFacesFromDirectory(const std::string& directory);
        }

    } // namespace Skybox
} // namespace Graphics