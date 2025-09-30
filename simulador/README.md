# Motor Gráfico OpenGL - Arquitectura Modular

## 📋 Descripción General

Este es un motor gráfico OpenGL moderno y profesional desarrollado en C++17 con una arquitectura completamente modular. El proyecto ha sido reestructurado desde una implementación monolítica a un diseño escalable y mantenible que sigue las mejores prácticas de ingeniería de software.

## 🏗️ Arquitectura del Proyecto

### Estructura de Directorios

```
ogl-project-template/
├── src/                           # Código fuente principal
│   ├── core/                      # Sistema central de OpenGL
│   │   ├── opengl_context.h       # Gestión del contexto OpenGL
│   │   └── opengl_context.cpp
│   ├── graphics/                  # Sistemas gráficos
│   │   ├── shaders/               # Gestión de shaders
│   │   │   ├── shader_manager.h
│   │   │   └── shader_manager.cpp
│   │   ├── textures/              # Gestión de texturas
│   │   │   ├── texture_manager.h
│   │   │   └── texture_manager.cpp
│   │   └── rendering/             # Objetos de renderizado
│   │       ├── buffer_objects.h
│   │       └── buffer_objects.cpp
│   ├── scene/                     # Sistema de escena
│   │   ├── mesh.h                 # Mallas y geometría
│   │   ├── mesh.cpp
│   │   ├── camera.h               # Sistema de cámara
│   │   └── camera.cpp
│   ├── input/                     # Gestión de entrada
│   │   ├── input_manager.h
│   │   └── input_manager.cpp
│   └── main.cpp                   # Aplicación principal
├── shaders/                       # Archivos de shaders GLSL
│   ├── vertex.glsl
│   ├── fragment.glsl
│   ├── vertex_3d.glsl
│   └── fragment_3d.glsl
├── textures/                      # Recursos de texturas
│   └── container.jpg
├── include/                       # Headers de librerías externas
│   ├── glad/
│   └── stb_image.h
├── build/                         # Archivos compilados
├── Makefile.simple               # Sistema de construcción simplificado
├── Makefile                      # Sistema de construcción principal
└── README.md                     # Esta documentación
```

## 🔧 Módulos del Sistema

### 1. Core (Graphics::Core)
**Responsabilidad:** Gestión del contexto OpenGL y ventana principal

**Componentes:**
- `OpenGLContext`: Inicialización y gestión de GLFW/OpenGL
- Configuración de viewport y callbacks
- Manejo de errores OpenGL

### 2. Graphics Systems

#### Shaders (Graphics::Shaders)
**Responsabilidad:** Compilación y gestión de shaders GLSL

**Características:**
- Sistema singleton para gestión centralizada
- Compilación automática de shaders
- Cache de shaders compilados
- Helpers para uniforms de GLM

#### Textures (Graphics::Textures)
**Responsabilidad:** Carga y gestión de texturas

**Características:**
- Soporte para texturas 2D y cubemaps
- Generación de texturas procedurales
- Gestión automática de memoria con RAII
- Integración con STB_IMAGE

#### Rendering (Graphics::Rendering)
**Responsabilidad:** Objetos de buffer OpenGL moderno

**Características:**
- VAO/VBO/EBO con RAII
- Gestión automática de recursos
- Interfaz tipo-segura para atributos

### 3. Scene System

#### Mesh (Scene::)
**Responsabilidad:** Representación y creación de geometría

**Características:**
- Factory pattern para geometría procedural
- Soporte para cubo, esfera, plano, skybox
- Integración con sistema de buffers
- Optimización de memoria

#### Camera (Scene::)
**Responsabilidad:** Sistema de cámara multipropósito

**Características:**
- Múltiples tipos: FPS, Orbital, Ortográfica
- Controller unificado para múltiples cámaras
- Matrices de vista y proyección optimizadas
- Frustum culling integrado

### 4. Input System (Input::)
**Responsabilidad:** Gestión unificada de entrada

**Características:**
- Tracking de estado de teclado y mouse
- Sistema de callbacks personalizable
- Gestión de acciones de alto nivel
- Integración con sistemas de cámara

## 🚀 Construcción y Ejecución

### Requisitos del Sistema
- **Compilador:** GCC/Clang con soporte C++17
- **Librerías:** OpenGL, GLFW, GLM
- **Sistema:** Linux/Windows/macOS

### Dependencias
```bash
# Ubuntu/Debian
sudo apt install libglfw3-dev libglm-dev libgl1-mesa-dev

# Fedora
sudo dnf install glfw-devel glm-devel mesa-libGL-devel

# Arch Linux  
sudo pacman -S glfw glm mesa
```

### Compilación
```bash
# Usar Makefile simplificado (recomendado)
make -f Makefile.simple

# O usar Makefile principal
make

# Limpiar archivos de construcción
make clean -f Makefile.simple
```

### Ejecución
```bash
./build/OGL-Engine
```

## 🎮 Controles

- **WASD:** Movimiento de cámara
- **Mouse:** Rotación de vista
- **Scroll:** Zoom
- **ESC:** Salir de la aplicación

## 🔬 Patrones de Diseño Implementados

### 1. Singleton Pattern
- **ShaderManager:** Gestión centralizada de shaders
- **TextureManager:** Cache global de texturas
- **InputManager:** Estado global de entrada

### 2. Factory Pattern
- **MeshFactory:** Creación de geometría procedural
- Facilita extensión con nuevos tipos de malla

### 3. RAII (Resource Acquisition Is Initialization)
- **Buffer Objects:** Gestión automática de VAO/VBO/EBO
- **Texture:** Liberación automática de memoria GPU
- **Camera:** Limpieza automática de recursos

### 4. Namespace Organization
- Separación lógica por responsabilidades
- Prevención de conflictos de nombres
- Organización jerárquica clara

## 🏆 Beneficios de la Arquitectura Modular

### ✅ Mantenibilidad
- **Separación clara de responsabilidades**
- **Código más legible y organizado**
- **Fácil localización de bugs**
- **Estructura escalable para proyectos grandes**

### ✅ Extensibilidad  
- **Nuevos sistemas se integran fácilmente**
- **Modificaciones aisladas por módulo**
- **Interfaz consistente entre componentes**
- **Soporte para múltiples implementaciones**

### ✅ Reutilización
- **Componentes independientes**
- **Interfaces bien definidas**
- **Sistemas plug-and-play**
- **Facilita testing unitario**

### ✅ Performance
- **Gestión eficiente de recursos**
- **Cache de assets optimizado**
- **Minimal overhead por abstracción**
- **Move semantics para performance**

## 🛠️ Configuración del Proyecto

### Variables de Compilación
```makefile
CXX = g++
CXXFLAGS = -std=c++17 -g -Wall -Wextra
INCLUDES = -I./src -I./include -I/usr/include/glm
LIBS = -lglfw -lGL -lX11 -lpthread -lXrandr -ldl
```

### Estructura de Headers
- **Consistencia .h/.cpp:** Todos los módulos siguen el mismo patrón
- **Forward Declarations:** Minimiza dependencias circulares
- **Include Guards:** Prevención de inclusiones múltiples
- **Namespace Consistency:** Organización lógica de símbolos

## 📚 Guía de Desarrollo

### Agregar Nuevos Sistemas
1. Crear directorio en `src/`
2. Implementar header (.h) e implementación (.cpp)
3. Añadir namespace apropiado
4. Actualizar Makefile
5. Integrar con sistema principal

### Mejores Prácticas
- **Usar RAII para gestión de recursos**
- **Aplicar const-correctness**
- **Implementar move semantics donde sea apropiado**
- **Mantener interfaces mínimas y cohesivas**
- **Documentar APIs públicas**

### Extensiones Recomendadas
- **Audio System:** Gestión de sonido 3D
- **Physics Engine:** Integración con Bullet/Box2D
- **Asset Pipeline:** Carga asíncrona de recursos
- **Scripting:** Integración con Lua/Python
- **Networking:** Sistema multiplayer

## 🔍 Depuración y Testing

### Herramientas de Debug
```bash
# Compilar con símbolos de debug
make -f Makefile.simple DEBUG=1

# Usar con gdb
gdb ./build/OGL-Engine

# Análisis de memoria con valgrind
valgrind --leak-check=full ./build/OGL-Engine
```

### Logging System
- OpenGL error checking automático
- Callbacks de debug integrados
- Niveles de log configurables

## 📈 Métricas de Mejora

### Antes (Monolítico)
- ❌ 285 líneas en un solo archivo
- ❌ Headers inconsistentes (.h vs solo header)
- ❌ Gestión manual de recursos
- ❌ Variables globales dispersas
- ❌ Lógica mezclada sin separación

### Después (Modular)
- ✅ 8 módulos especializados bien definidos
- ✅ Consistencia total .h/.cpp en todos los archivos
- ✅ RAII automático para todos los recursos
- ✅ Estado encapsulado en clases singleton
- ✅ Separación clara de responsabilidades

## 🎯 Resultados del Refactoring

Este refactoring completo ha transformado un proyecto OpenGL básico en un **motor gráfico profesional y escalable**. La nueva arquitectura modular no solo cumple con los estándares de la industria, sino que establece una base sólida para el desarrollo de aplicaciones gráficas complejas.

### Impacto en el Desarrollo
- **Tiempo de desarrollo:** Reducido significativamente para nuevas funcionalidades
- **Debugging:** Localización de errores mucho más eficiente  
- **Colaboración:** Múltiples desarrolladores pueden trabajar en paralelo
- **Calidad:** Código más robusto y menos propenso a errores

---

**Desarrollado con ❤️ usando C++17 y OpenGL 3.3+**