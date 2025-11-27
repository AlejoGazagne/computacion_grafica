#pragma once

#include <glad/glad.h>
#include <iostream>

namespace Graphics
{
  namespace Rendering
  {

    /**
     * @brief Clase que maneja el Shadow Mapping (Depth Map)
     *
     * Basado en la técnica de Shadow Mapping vista en teoría:
     * 1. Renderizar la escena desde la perspectiva de la luz a un depth buffer
     * 2. Usar ese depth buffer como textura para determinar qué píxeles están en sombra
     *
     * Referencias: OpenGL 4.0 Shading Language Cookbook - Cap. Shadow Mapping
     */
    class ShadowMap
    {
    private:
      GLuint fbo_;          // Framebuffer Object
      GLuint depth_map_;    // Textura de profundidad
      unsigned int width_;  // Ancho del shadow map
      unsigned int height_; // Alto del shadow map

    public:
      /**
       * @brief Constructor - crea el FBO y la textura de profundidad
       * @param width Resolución en X del shadow map (ej: 4096)
       * @param height Resolución en Y del shadow map (ej: 4096)
       */
      ShadowMap(unsigned int width = 4096, unsigned int height = 4096)
          : fbo_(0), depth_map_(0), width_(width), height_(height)
      {
        initialize();
      }

      ~ShadowMap()
      {
        cleanup();
      }

      /**
       * @brief Inicializa el FBO y la textura de profundidad
       */
      bool initialize()
      {
        // Crear el Framebuffer Object
        glGenFramebuffers(1, &fbo_);

        // Crear la textura de profundidad
        glGenTextures(1, &depth_map_);
        glBindTexture(GL_TEXTURE_2D, depth_map_);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
                     width_, height_, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

        // Parámetros de la textura de profundidad
        // GL_NEAREST para evitar interpolación de valores de profundidad
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        // GL_CLAMP_TO_BORDER para que fuera del frustum de luz no haya sombras
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        float border_color[] = {1.0f, 1.0f, 1.0f, 1.0f}; // Blanco = sin sombra
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border_color);

        // Attachar la textura de profundidad al FBO
        glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                               GL_TEXTURE_2D, depth_map_, 0);

        // No necesitamos color buffer, solo profundidad
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);

        // Verificar que el FBO está completo
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
          std::cerr << "ERROR: Shadow Map Framebuffer is not complete!" << std::endl;
          glBindFramebuffer(GL_FRAMEBUFFER, 0);
          return false;
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        std::cout << "Shadow Map initialized: " << width_ << "x" << height_ << std::endl;
        return true;
      }

      /**
       * @brief Activa el shadow map para escritura (shadow pass)
       */
      void bindForWriting()
      {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
        glViewport(0, 0, width_, height_);
        glClear(GL_DEPTH_BUFFER_BIT);
      }

      /**
       * @brief Activa el shadow map para lectura (render pass)
       * @param texture_unit Unidad de textura a usar (ej: GL_TEXTURE0 + 5)
       */
      void bindForReading(GLenum texture_unit)
      {
        glActiveTexture(texture_unit);
        glBindTexture(GL_TEXTURE_2D, depth_map_);
      }

      /**
       * @brief Desactiva el FBO y restaura el default framebuffer
       */
      void unbind()
      {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
      }

      /**
       * @brief Obtiene el ID de la textura de profundidad
       */
      GLuint getDepthMap() const
      {
        return depth_map_;
      }

      /**
       * @brief Obtiene las dimensiones del shadow map
       */
      void getSize(unsigned int &width, unsigned int &height) const
      {
        width = width_;
        height = height_;
      }

    private:
      void cleanup()
      {
        if (depth_map_ != 0)
        {
          glDeleteTextures(1, &depth_map_);
          depth_map_ = 0;
        }
        if (fbo_ != 0)
        {
          glDeleteFramebuffers(1, &fbo_);
          fbo_ = 0;
        }
      }

      // Prevenir copia
      ShadowMap(const ShadowMap &) = delete;
      ShadowMap &operator=(const ShadowMap &) = delete;
    };

  } // namespace Rendering
} // namespace Graphics
