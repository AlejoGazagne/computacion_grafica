# OpenGL Graphics Engine - Modular Architecture

## 🎯 Arquitectura Completamente Refactorizada

Este proyecto ha sido completamente restructurado desde una aplicación monolítica a una **arquitectura modular profesional** con separación clara de responsabilidades.

## 📁 Nueva Estructura de Proyecto

```
src/
├── core/                    # Sistema núcleo
│   ├── opengl_context.h/cpp # Gestión de contexto OpenGL
│   └── ...
├── graphics/                # Sistemas gráficos
│   ├── shaders/            # Gestión de shaders
│   │   └── shader_manager.h/cpp
│   ├── textures/           # Gestión de texturas
│   │   └── texture_manager.h/cpp
│   └── rendering/          # Sistema de renderizado
│       └── buffer_objects.h/cpp
├── scene/                  # Sistema de escena
│   ├── mesh.h/cpp         # Geometría y meshes
│   └── camera.h/cpp       # Sistema de cámara
├── input/                 # Sistema de entrada
│   └── input_manager.h/cpp
└── utils/                 # Utilidades
    └── utils.h/cpp
```

## 🚀 Características Principales

### ✅ Sistemas Modulares Implementados

1. **Core System**
   - `OpenGLContext`: Gestión profesional de contexto OpenGL/GLFW
   - Configuración de ventana, callbacks automáticos
   - Gestión de errores y información del contexto

2. **Graphics Systems**
   - `ShaderManager`: Singleton para gestión de shaders con cache
   - `TextureManager`: Gestión completa de texturas 2D y cubemaps
   - `BufferObjects`: VAO, VBO, EBO modernos con RAII

3. **Scene System**
   - `MeshFactory`: Factory pattern para geometría procedural
   - `Camera`: Sistema de cámara con múltiples modos (FPS, Orbital, etc.)
   - `CameraController`: Gestión de múltiples cámaras

4. **Input System**
   - `InputManager`: Sistema unificado de entrada con callbacks
   - `ActionManager`: Mapeo de acciones de alto nivel
   - Gestión de estado de teclas y mouse

### ✅ Nuevas Funcionalidades

- **Skybox System** (según petición original)
  - Soporte completo para cubemaps
  - `TextureCubemap` con `FaceTexture`
  - Mesh especial para skybox con matriz de vista sin traslación
  - Gestión correcta de depth mask y face culling

- **Arquitectura RAII**
  - Gestión automática de recursos OpenGL
  - Sin memory leaks por diseño
  - Move semantics para objetos pesados

- **Sistema de Build Modular**
  - Makefile actualizado para compilación por módulos
  - Targets específicos para cada sistema
  - Configuraciones de debug/release

## 🎮 Controles

| Tecla | Acción |
|-------|---------|
| WASD | Mover cámara |
| QE | Subir/Bajar |
| Mouse | Mirar alrededor |
| Scroll | Zoom |
| G | Toggle wireframe |
| T | Toggle textura |
| R | Reset cámara |
| E | Toggle captura de mouse |
| 1 | Mostrar controles |
| ESC | Salir |

## 🛠 Compilación

```bash
# Limpiar y compilar
make clean && make

# Ver información del build
make info

# Compilar solo módulos
make modules

# Limpiar solo módulos
make clean-modules
```

## 📚 Uso de la Nueva Arquitectura

### Ejemplo: Crear un Skybox

```cpp
// Cargar texturas del skybox
std::vector<FaceTexture> skybox_faces = {
    {"textures/skybox/right.jpg", CubeFace::POSITIVE_X},
    {"textures/skybox/left.jpg", CubeFace::NEGATIVE_X},
    {"textures/skybox/top.jpg", CubeFace::POSITIVE_Y},
    {"textures/skybox/bottom.jpg", CubeFace::NEGATIVE_Y},
    {"textures/skybox/front.jpg", CubeFace::POSITIVE_Z},
    {"textures/skybox/back.jpg", CubeFace::NEGATIVE_Z}
};

auto& texture_manager = TextureManager::getInstance();
texture_manager.loadCubemap("skybox", skybox_faces);

// Crear mesh del skybox
auto skybox_mesh = MeshFactory::createSkyboxCube();

// En el loop de renderizado:
glDepthMask(GL_FALSE);
skybox_shader->use();
skybox_texture->bind();
skybox_shader->setMat4("view", camera->getViewMatrixNoTranslation());
skybox_mesh->draw();
glDepthMask(GL_TRUE);
```

### Ejemplo: Gestión de Shaders

```cpp
auto& shader_manager = ShaderManager::getInstance();

// Cargar shader
shader_manager.loadShader("basic", "shaders/vertex.glsl", "shaders/fragment.glsl");

// Usar shader
Shader* shader = shader_manager.getShader("basic");
shader->use();
shader->setMat4("mvpMatrix", mvp);
shader->setVec3("lightPos", light_position);
```

### Ejemplo: Sistema de Entrada

```cpp
auto& input_manager = InputManager::getInstance();

// Configurar callbacks
input_manager.addKeyCallback([](int key, KeyState state, float dt) {
    if (key == InputManager::KEY_SPACE && state == KeyState::JUST_PRESSED) {
        // Acción al presionar espacio
    }
});

// En el loop principal
input_manager.update(delta_time);
```

## 🔄 Migración desde la Versión Antigua

La versión anterior (main_old.cpp) usaba un enfoque monolítico. La nueva arquitectura ofrece:

- **Mejor organización**: Cada sistema tiene su responsabilidad
- **Escalabilidad**: Fácil agregar nuevas funcionalidades
- **Mantenibilidad**: Código más limpio y modular
- **Rendimiento**: Gestión optimizada de recursos
- **Flexibilidad**: Sistema de configuración por módulos

## 🏗 Próximas Extensiones

La arquitectura actual permite fácilmente agregar:

- Sistema de materiales
- Sistema de luces
- Sistema de partículas  
- Sistema de audio
- Sistema de física
- Sistema de scripting
- Sistema de recursos (assets)
- Sistema de escenas múltiples

## 📝 Notas de Desarrollo

- Todos los archivos .h tienen su correspondiente .cpp
- Uso de namespaces para organización (`Graphics::`, `Scene::`, etc.)
- Patrón Singleton para managers globales
- Factory patterns para creación de objetos
- RAII para gestión de recursos
- Move semantics para optimización

---

**¡La nueva arquitectura está lista para usarse como base de proyectos gráficos serios!** 🚀