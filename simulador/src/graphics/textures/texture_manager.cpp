#include "texture_manager.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../../include/stb_image.h"

#include <iostream>
#include <filesystem>
#include <algorithm>

namespace Graphics {
    namespace Textures {

        // Inicialización del singleton
        std::unique_ptr<TextureManager> TextureManager::instance_ = nullptr;

        // === Implementación de Texture ===

        Texture::Texture() 
            : texture_id_(0), type_(TextureType::TEXTURE_2D), format_(TextureFormat::RGBA),
              width_(0), height_(0), channels_(0), name_(""), loaded_(false),
              min_filter_(TextureFilter::LINEAR), mag_filter_(TextureFilter::LINEAR),
              wrap_s_(TextureWrap::REPEAT), wrap_t_(TextureWrap::REPEAT), wrap_r_(TextureWrap::REPEAT) {
        }

        Texture::Texture(const std::string& name, TextureType type)
            : texture_id_(0), type_(type), format_(TextureFormat::RGBA),
              width_(0), height_(0), channels_(0), name_(name), loaded_(false),
              min_filter_(TextureFilter::LINEAR), mag_filter_(TextureFilter::LINEAR),
              wrap_s_(TextureWrap::REPEAT), wrap_t_(TextureWrap::REPEAT), wrap_r_(TextureWrap::REPEAT) {
        }

        Texture::~Texture() {
            if (texture_id_ != 0) {
                glDeleteTextures(1, &texture_id_);
            }
        }

        Texture::Texture(Texture&& other) noexcept
            : texture_id_(other.texture_id_), type_(other.type_), format_(other.format_),
              width_(other.width_), height_(other.height_), channels_(other.channels_),
              name_(std::move(other.name_)), loaded_(other.loaded_),
              min_filter_(other.min_filter_), mag_filter_(other.mag_filter_),
              wrap_s_(other.wrap_s_), wrap_t_(other.wrap_t_), wrap_r_(other.wrap_r_) {
            
            other.texture_id_ = 0;
            other.loaded_ = false;
        }

        Texture& Texture::operator=(Texture&& other) noexcept {
            if (this != &other) {
                if (texture_id_ != 0) {
                    glDeleteTextures(1, &texture_id_);
                }
                
                texture_id_ = other.texture_id_;
                type_ = other.type_;
                format_ = other.format_;
                width_ = other.width_;
                height_ = other.height_;
                channels_ = other.channels_;
                name_ = std::move(other.name_);
                loaded_ = other.loaded_;
                min_filter_ = other.min_filter_;
                mag_filter_ = other.mag_filter_;
                wrap_s_ = other.wrap_s_;
                wrap_t_ = other.wrap_t_;
                wrap_r_ = other.wrap_r_;
                
                other.texture_id_ = 0;
                other.loaded_ = false;
            }
            return *this;
        }

        void Texture::applyParameters() {
            GLenum target = (type_ == TextureType::TEXTURE_2D) ? GL_TEXTURE_2D : GL_TEXTURE_CUBE_MAP;
            
            glBindTexture(target, texture_id_);
            
            glTexParameteri(target, GL_TEXTURE_MIN_FILTER, static_cast<GLint>(min_filter_));
            glTexParameteri(target, GL_TEXTURE_MAG_FILTER, static_cast<GLint>(mag_filter_));
            glTexParameteri(target, GL_TEXTURE_WRAP_S, static_cast<GLint>(wrap_s_));
            glTexParameteri(target, GL_TEXTURE_WRAP_T, static_cast<GLint>(wrap_t_));
            
            if (type_ == TextureType::TEXTURE_CUBE_MAP) {
                glTexParameteri(target, GL_TEXTURE_WRAP_R, static_cast<GLint>(wrap_r_));
            }
            
            glBindTexture(target, 0);
        }

        bool Texture::loadImageData(const std::string& filepath, unsigned char*& data,
                                  int& width, int& height, int& channels, bool flip_vertically) {
            stbi_set_flip_vertically_on_load(flip_vertically);
            
            data = stbi_load(filepath.c_str(), &width, &height, &channels, 0);
            
            if (!data) {
                std::cerr << "ERROR: Failed to load texture: " << filepath << std::endl;
                std::cerr << "STB Error: " << stbi_failure_reason() << std::endl;
                return false;
            }
            
            return true;
        }

        bool Texture::loadFromFile(const std::string& filepath, bool flip_vertically) {
            if (type_ != TextureType::TEXTURE_2D) {
                std::cerr << "ERROR: loadFromFile only works with TEXTURE_2D" << std::endl;
                return false;
            }

            unsigned char* data;
            if (!loadImageData(filepath, data, width_, height_, channels_, flip_vertically)) {
                return false;
            }

            // Generar textura
            glGenTextures(1, &texture_id_);
            glBindTexture(GL_TEXTURE_2D, texture_id_);

            // Determinar formato
            GLenum format, internal_format;
            if (channels_ == 1) {
                format = internal_format = GL_RED;
            } else if (channels_ == 3) {
                format = internal_format = GL_RGB;
            } else if (channels_ == 4) {
                format = internal_format = GL_RGBA;
            } else {
                std::cerr << "ERROR: Unsupported channel count: " << channels_ << std::endl;
                stbi_image_free(data);
                return false;
            }

            format_ = static_cast<TextureFormat>(format);

            // Cargar datos de textura
            glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width_, height_, 0, format, GL_UNSIGNED_BYTE, data);
            
            // Generar mipmaps
            generateMipmaps();
            
            // Aplicar parámetros
            applyParameters();
            
            stbi_image_free(data);
            glBindTexture(GL_TEXTURE_2D, 0);
            
            loaded_ = true;
            std::cout << "Texture loaded: " << filepath << " (" << width_ << "x" << height_ 
                      << ", " << channels_ << " channels)" << std::endl;
            
            return true;
        }

        bool Texture::createProcedural(int width, int height, 
                                     unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
            if (type_ != TextureType::TEXTURE_2D) {
                std::cerr << "ERROR: createProcedural only works with TEXTURE_2D" << std::endl;
                return false;
            }

            width_ = width;
            height_ = height;
            channels_ = 4;
            format_ = TextureFormat::RGBA;

            // Crear datos de textura procedural
            std::vector<unsigned char> data(width * height * 4);
            for (int i = 0; i < width * height; ++i) {
                data[i * 4 + 0] = r;
                data[i * 4 + 1] = g;
                data[i * 4 + 2] = b;
                data[i * 4 + 3] = a;
            }

            // Generar textura
            glGenTextures(1, &texture_id_);
            glBindTexture(GL_TEXTURE_2D, texture_id_);
            
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_, height_, 0, GL_RGBA, GL_UNSIGNED_BYTE, data.data());
            
            generateMipmaps();
            applyParameters();
            
            glBindTexture(GL_TEXTURE_2D, 0);
            
            loaded_ = true;
            std::cout << "Procedural texture created: " << width_ << "x" << height_ << std::endl;
            
            return true;
        }

        bool Texture::createEmpty(int width, int height, TextureFormat format) {
            if (type_ != TextureType::TEXTURE_2D) {
                std::cerr << "ERROR: createEmpty only works with TEXTURE_2D" << std::endl;
                return false;
            }

            width_ = width;
            height_ = height;
            format_ = format;

            glGenTextures(1, &texture_id_);
            glBindTexture(GL_TEXTURE_2D, texture_id_);
            
            glTexImage2D(GL_TEXTURE_2D, 0, static_cast<GLenum>(format), width, height, 0, 
                        static_cast<GLenum>(format), GL_UNSIGNED_BYTE, nullptr);
            
            applyParameters();
            glBindTexture(GL_TEXTURE_2D, 0);
            
            loaded_ = true;
            return true;
        }

        bool Texture::loadCubemapFromFiles(const std::vector<FaceTexture>& face_textures) {
            if (type_ != TextureType::TEXTURE_CUBE_MAP) {
                std::cerr << "ERROR: loadCubemapFromFiles only works with TEXTURE_CUBE_MAP" << std::endl;
                return false;
            }

            if (face_textures.size() != 6) {
                std::cerr << "ERROR: Cubemap requires exactly 6 faces" << std::endl;
                return false;
            }

            glGenTextures(1, &texture_id_);
            glBindTexture(GL_TEXTURE_CUBE_MAP, texture_id_);

            bool success = true;
            for (const auto& face_texture : face_textures) {
                unsigned char* data;
                int width, height, channels;
                
                if (!loadImageData(face_texture.filepath, data, width, height, channels, face_texture.flip_vertically)) {
                    success = false;
                    break;
                }

                // Verificar que todas las caras tengan el mismo tamaño
                if (width_ == 0) {
                    width_ = width;
                    height_ = height;
                    channels_ = channels;
                } else if (width_ != width || height_ != height) {
                    std::cerr << "ERROR: All cubemap faces must have the same dimensions" << std::endl;
                    stbi_image_free(data);
                    success = false;
                    break;
                }

                // Determinar formato
                GLenum format;
                if (channels == 3) format = GL_RGB;
                else if (channels == 4) format = GL_RGBA;
                else {
                    std::cerr << "ERROR: Unsupported channel count for cubemap: " << channels << std::endl;
                    stbi_image_free(data);
                    success = false;
                    break;
                }

                glTexImage2D(static_cast<GLenum>(face_texture.face), 0, format, width, height, 0, 
                            format, GL_UNSIGNED_BYTE, data);
                
                stbi_image_free(data);
            }

            if (success) {
                format_ = channels_ == 3 ? TextureFormat::RGB : TextureFormat::RGBA;
                
                // Configuración específica para cubemaps
                setMinFilter(TextureFilter::LINEAR);
                setMagFilter(TextureFilter::LINEAR);
                setWrapS(TextureWrap::CLAMP_TO_EDGE);
                setWrapT(TextureWrap::CLAMP_TO_EDGE);
                setWrapR(TextureWrap::CLAMP_TO_EDGE);
                
                applyParameters();
                loaded_ = true;
                
                std::cout << "Cubemap loaded successfully (" << width_ << "x" << height_ << ")" << std::endl;
            } else {
                if (texture_id_ != 0) {
                    glDeleteTextures(1, &texture_id_);
                    texture_id_ = 0;
                }
            }

            glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
            return success;
        }

        bool Texture::loadCubemapFromDirectory(const std::string& directory, const std::string& extension) {
            // Orden convencional: right, left, top, bottom, front, back
            std::vector<std::string> face_names = {"right", "left", "top", "bottom", "front", "back"};
            std::vector<CubeFace> faces = {
                CubeFace::POSITIVE_X, CubeFace::NEGATIVE_X,
                CubeFace::POSITIVE_Y, CubeFace::NEGATIVE_Y,
                CubeFace::POSITIVE_Z, CubeFace::NEGATIVE_Z
            };

            std::vector<FaceTexture> face_textures;
            
            for (size_t i = 0; i < 6; ++i) {
                std::string filepath = directory + "/" + face_names[i] + "." + extension;
                FaceTexture face_tex;
                face_tex.filepath = filepath;
                face_tex.face = faces[i];
                face_tex.flip_vertically = false; // Cubemaps normalmente no se voltean
                face_textures.push_back(face_tex);
            }

            return loadCubemapFromFiles(face_textures);
        }

        void Texture::bind(unsigned int unit) const {
            if (!loaded_) return;
            
            glActiveTexture(GL_TEXTURE0 + unit);
            
            if (type_ == TextureType::TEXTURE_2D) {
                glBindTexture(GL_TEXTURE_2D, texture_id_);
            } else if (type_ == TextureType::TEXTURE_CUBE_MAP) {
                glBindTexture(GL_TEXTURE_CUBE_MAP, texture_id_);
            }
        }

        void Texture::unbind() const {
            if (type_ == TextureType::TEXTURE_2D) {
                glBindTexture(GL_TEXTURE_2D, 0);
            } else if (type_ == TextureType::TEXTURE_CUBE_MAP) {
                glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
            }
        }

        void Texture::setMinFilter(TextureFilter filter) {
            min_filter_ = filter;
            if (loaded_) applyParameters();
        }

        void Texture::setMagFilter(TextureFilter filter) {
            mag_filter_ = filter;
            if (loaded_) applyParameters();
        }

        void Texture::setWrapS(TextureWrap wrap) {
            wrap_s_ = wrap;
            if (loaded_) applyParameters();
        }

        void Texture::setWrapT(TextureWrap wrap) {
            wrap_t_ = wrap;
            if (loaded_) applyParameters();
        }

        void Texture::setWrapR(TextureWrap wrap) {
            wrap_r_ = wrap;
            if (loaded_) applyParameters();
        }

        void Texture::setBorderColor(float r, float g, float b, float a) {
            if (!loaded_) return;
            
            float color[] = {r, g, b, a};
            GLenum target = (type_ == TextureType::TEXTURE_2D) ? GL_TEXTURE_2D : GL_TEXTURE_CUBE_MAP;
            
            glBindTexture(target, texture_id_);
            glTexParameterfv(target, GL_TEXTURE_BORDER_COLOR, color);
            glBindTexture(target, 0);
        }

        void Texture::generateMipmaps() {
            if (!loaded_) return;
            
            GLenum target = (type_ == TextureType::TEXTURE_2D) ? GL_TEXTURE_2D : GL_TEXTURE_CUBE_MAP;
            glGenerateMipmap(target);
        }

        // === Implementación de TextureManager ===

        TextureManager& TextureManager::getInstance() {
            if (!instance_) {
                instance_ = std::make_unique<TextureManager>();
            }
            return *instance_;
        }

        bool TextureManager::loadTexture2D(const std::string& name, const std::string& filepath, bool flip_vertically) {
            auto texture = std::make_unique<Texture>(name, TextureType::TEXTURE_2D);
            
            if (!texture->loadFromFile(filepath, flip_vertically)) {
                std::cerr << "Failed to load 2D texture: " << name << std::endl;
                return false;
            }
            
            textures_[name] = std::move(texture);
            std::cout << "2D Texture registered: " << name << std::endl;
            return true;
        }

        bool TextureManager::loadCubemap(const std::string& name, const std::vector<FaceTexture>& face_textures) {
            auto texture = std::make_unique<Texture>(name, TextureType::TEXTURE_CUBE_MAP);
            
            if (!texture->loadCubemapFromFiles(face_textures)) {
                std::cerr << "Failed to load cubemap: " << name << std::endl;
                return false;
            }
            
            textures_[name] = std::move(texture);
            std::cout << "Cubemap registered: " << name << std::endl;
            return true;
        }

        bool TextureManager::loadCubemap(const std::string& name, const std::string& directory, const std::string& extension) {
            auto texture = std::make_unique<Texture>(name, TextureType::TEXTURE_CUBE_MAP);
            
            if (!texture->loadCubemapFromDirectory(directory, extension)) {
                std::cerr << "Failed to load cubemap from directory: " << name << std::endl;
                return false;
            }
            
            textures_[name] = std::move(texture);
            std::cout << "Cubemap from directory registered: " << name << std::endl;
            return true;
        }

        bool TextureManager::createProceduralTexture(const std::string& name, int width, int height,
                                                   unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
            auto texture = std::make_unique<Texture>(name, TextureType::TEXTURE_2D);
            
            if (!texture->createProcedural(width, height, r, g, b, a)) {
                std::cerr << "Failed to create procedural texture: " << name << std::endl;
                return false;
            }
            
            textures_[name] = std::move(texture);
            std::cout << "Procedural texture registered: " << name << std::endl;
            return true;
        }

        Texture* TextureManager::getTexture(const std::string& name) {
            auto it = textures_.find(name);
            if (it != textures_.end()) {
                return it->second.get();
            }
            
            std::cerr << "Texture not found: " << name << std::endl;
            return nullptr;
        }

        void TextureManager::removeTexture(const std::string& name) {
            textures_.erase(name);
        }

        void TextureManager::clear() {
            textures_.clear();
        }

        bool TextureManager::hasTexture(const std::string& name) const {
            return textures_.find(name) != textures_.end();
        }

        size_t TextureManager::getTextureCount() const {
            return textures_.size();
        }

    } // namespace Textures
} // namespace Graphics