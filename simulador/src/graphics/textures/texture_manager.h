#ifndef TEXTURE_MANAGER_H
#define TEXTURE_MANAGER_H

extern "C" {
    #include <glad/glad.h>
}

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

namespace Graphics {
    namespace Textures {

        enum class TextureType {
            TEXTURE_2D,
            TEXTURE_CUBE_MAP
        };

        enum class TextureFormat {
            RGB = GL_RGB,
            RGBA = GL_RGBA,
            DEPTH = GL_DEPTH_COMPONENT,
            DEPTH_STENCIL = GL_DEPTH_STENCIL
        };

        enum class TextureFilter {
            LINEAR = GL_LINEAR,
            NEAREST = GL_NEAREST,
            LINEAR_MIPMAP_LINEAR = GL_LINEAR_MIPMAP_LINEAR,
            NEAREST_MIPMAP_NEAREST = GL_NEAREST_MIPMAP_NEAREST
        };

        enum class TextureWrap {
            REPEAT = GL_REPEAT,
            MIRRORED_REPEAT = GL_MIRRORED_REPEAT,
            CLAMP_TO_EDGE = GL_CLAMP_TO_EDGE,
            CLAMP_TO_BORDER = GL_CLAMP_TO_BORDER
        };

        enum class CubeFace {
            POSITIVE_X = GL_TEXTURE_CUBE_MAP_POSITIVE_X,
            NEGATIVE_X = GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
            POSITIVE_Y = GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
            NEGATIVE_Y = GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
            POSITIVE_Z = GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
            NEGATIVE_Z = GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
        };

        struct FaceTexture {
            std::string filepath;
            CubeFace face;
            bool flip_vertically = false;
        };

        class Texture {
        private:
            GLuint texture_id_;
            TextureType type_;
            TextureFormat format_;
            int width_, height_, channels_;
            std::string name_;
            bool loaded_;
            
            // Configuraciones
            TextureFilter min_filter_;
            TextureFilter mag_filter_;
            TextureWrap wrap_s_;
            TextureWrap wrap_t_;
            TextureWrap wrap_r_;

            void applyParameters();

        public:
            Texture();
            Texture(const std::string& name, TextureType type = TextureType::TEXTURE_2D);
            ~Texture();

            // No permitir copia
            Texture(const Texture&) = delete;
            Texture& operator=(const Texture&) = delete;

            // Permitir movimiento
            Texture(Texture&& other) noexcept;
            Texture& operator=(Texture&& other) noexcept;

            // Cargar desde archivo (2D)
            bool loadFromFile(const std::string& filepath, bool flip_vertically = true);
            
            // Crear textura procedural (2D)
            bool createProcedural(int width, int height, 
                                unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255);
            
            // Crear textura en blanco
            bool createEmpty(int width, int height, TextureFormat format = TextureFormat::RGBA);
            
            // Cargar cubemap desde archivos
            bool loadCubemapFromFiles(const std::vector<FaceTexture>& face_textures);
            
            // Cargar cubemap desde directorio (convención: right, left, top, bottom, front, back)
            bool loadCubemapFromDirectory(const std::string& directory, 
                                        const std::string& extension = "jpg");

            void bind(unsigned int unit = 0) const;
            void unbind() const;

            // Getters
            GLuint getId() const { return texture_id_; }
            TextureType getType() const { return type_; }
            TextureFormat getFormat() const { return format_; }
            int getWidth() const { return width_; }
            int getHeight() const { return height_; }
            int getChannels() const { return channels_; }
            const std::string& getName() const { return name_; }
            bool isLoaded() const { return loaded_; }

            // Setters para configuración
            void setMinFilter(TextureFilter filter);
            void setMagFilter(TextureFilter filter);
            void setWrapS(TextureWrap wrap);
            void setWrapT(TextureWrap wrap);
            void setWrapR(TextureWrap wrap);
            void setBorderColor(float r, float g, float b, float a);
            
            void generateMipmaps();

        private:
            bool loadImageData(const std::string& filepath, unsigned char*& data, 
                             int& width, int& height, int& channels, bool flip_vertically);
        };

        class TextureManager {
        private:
            std::unordered_map<std::string, std::unique_ptr<Texture>> textures_;
            static std::unique_ptr<TextureManager> instance_;

        public:
            TextureManager() = default;
            ~TextureManager() = default;

            // Singleton
            static TextureManager& getInstance();

            // No permitir copia
            TextureManager(const TextureManager&) = delete;
            TextureManager& operator=(const TextureManager&) = delete;

            // Cargar texturas 2D
            bool loadTexture2D(const std::string& name, const std::string& filepath, 
                             bool flip_vertically = true);
            
            // Cargar cubemap
            bool loadCubemap(const std::string& name, const std::vector<FaceTexture>& face_textures);
            bool loadCubemap(const std::string& name, const std::string& directory, 
                           const std::string& extension = "jpg");
            
            // Crear texturas procedurales
            bool createProceduralTexture(const std::string& name, int width, int height,
                                       unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255);

            Texture* getTexture(const std::string& name);
            void removeTexture(const std::string& name);
            void clear();
            
            // Utilidades
            bool hasTexture(const std::string& name) const;
            size_t getTextureCount() const;
        };

    } // namespace Textures
} // namespace Graphics

#endif // TEXTURE_MANAGER_H