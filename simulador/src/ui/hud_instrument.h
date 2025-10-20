#ifndef HUD_INSTRUMENT_H
#define HUD_INSTRUMENT_H

#include <glm/glm.hpp>
extern "C"
{
#include <glad/glad.h>
#include <GLFW/glfw3.h>
}
#include <string>
#include "graphics/shaders/shader_manager.h"

namespace UI
{

  /**
   * @brief Clase base abstracta para instrumentos del HUD
   *
   * Proporciona la interfaz común y gestión de recursos OpenGL compartida
   * para todos los instrumentos del HUD (bank angle, pitch ladder, etc.)
   *
   * Responsabilidades:
   * - Gestión de VAO/VBO
   * - Carga de shaders mediante ShaderManager
   * - Administración de dimensiones de pantalla
   * - Interfaz común para renderizado y actualización
   */
  class HUDInstrument
  {
  protected:
    // Recursos OpenGL compartidos
    GLuint VAO_;
    GLuint VBO_;
    std::string shader_name_;

    // Dimensiones de pantalla
    int screen_width_;
    int screen_height_;

    /**
     * @brief Inicializa recursos OpenGL específicos del instrumento
     *
     * Las clases derivadas pueden sobrescribir este método para personalizar
     * la inicialización, pero deben llamar a la implementación base.
     *
     * @param vertex_shader_path Ruta al vertex shader
     * @param fragment_shader_path Ruta al fragment shader
     * @return true si la inicialización fue exitosa
     */
    virtual bool initializeOpenGL(const std::string &vertex_shader_path,
                                  const std::string &fragment_shader_path);

    /**
     * @brief Libera recursos OpenGL
     *
     * Las clases derivadas pueden extender este método para limpiar
     * recursos adicionales, pero deben llamar a la implementación base.
     */
    virtual void cleanup();

  public:
    /**
     * @brief Constructor de la clase base
     * @param width Ancho de pantalla inicial
     * @param height Alto de pantalla inicial
     * @param shader_name Nombre único del shader para este instrumento
     */
    HUDInstrument(int width, int height, const std::string &shader_name);

    /**
     * @brief Destructor virtual para permitir polimorfismo
     */
    virtual ~HUDInstrument();

    // Deshabilitar copia (instrumentos manejan recursos OpenGL únicos)
    HUDInstrument(const HUDInstrument &) = delete;
    HUDInstrument &operator=(const HUDInstrument &) = delete;

    /**
     * @brief Actualiza las dimensiones de pantalla
     * @param width Nuevo ancho
     * @param height Nuevo alto
     */
    virtual void updateScreenSize(int width, int height);

    /**
     * @brief Método de renderizado abstracto - debe ser implementado por cada instrumento
     *
     * Cada instrumento decide qué parámetros necesita para renderizarse.
     * Este método será llamado por el HUD con los datos apropiados.
     */
    virtual void render() = 0;

    /**
     * @brief Verifica si el instrumento está correctamente inicializado
     * @return true si los recursos OpenGL están listos
     */
    bool isInitialized() const { return VAO_ != 0; }

    /**
     * @brief Obtiene el nombre del shader asociado a este instrumento
     * @return Nombre del shader
     */
    const std::string &getShaderName() const { return shader_name_; }
  };

} // namespace UI

#endif // HUD_INSTRUMENT_H
