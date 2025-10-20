#include "hud_instrument.h"
#include <iostream>

namespace UI
{

  HUDInstrument::HUDInstrument(int width, int height, const std::string &shader_name)
      : VAO_(0), VBO_(0), shader_name_(shader_name), screen_width_(width), screen_height_(height)
  {
  }

  HUDInstrument::~HUDInstrument()
  {
    cleanup();
  }

  bool HUDInstrument::initializeOpenGL(const std::string &vertex_shader_path,
                                       const std::string &fragment_shader_path)
  {
    // Usar ShaderManager para cargar shaders
    auto &shader_manager = Graphics::Shaders::ShaderManager::getInstance();

    if (!shader_manager.loadShader(shader_name_, vertex_shader_path, fragment_shader_path))
    {
      std::cerr << "Failed to load shaders for " << shader_name_ << std::endl;
      return false;
    }

    // Crear VAO y VBO
    glGenVertexArrays(1, &VAO_);
    glGenBuffers(1, &VBO_);

    glBindVertexArray(VAO_);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_);

    // Reservar buffer vacío inicial (los instrumentos lo llenarán dinámicamente)
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

    // Configurar atributo de posición (2D para HUD)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);

    glBindVertexArray(0);

    return true;
  }

  void HUDInstrument::cleanup()
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
    // Los shaders son manejados globalmente por ShaderManager, no los eliminamos aquí
  }

  void HUDInstrument::updateScreenSize(int width, int height)
  {
    screen_width_ = width;
    screen_height_ = height;
  }

} // namespace UI
