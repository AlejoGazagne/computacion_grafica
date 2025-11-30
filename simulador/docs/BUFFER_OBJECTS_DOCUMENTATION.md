# Sistema de Abstracción de Buffer Objects

Este documento detalla la implementación y uso de las clases de abstracción de OpenGL (`Buffer`, `VertexBuffer`, `IndexBuffer`, `VertexArray`) desarrolladas para el simulador de vuelo. Estas clases encapsulan la gestión de memoria de la GPU y simplifican el pipeline de renderizado.

## 1. Propósito y Motivación

OpenGL es una API de bajo nivel escrita en C que utiliza identificadores enteros (`GLuint`) para manejar recursos. Esto presenta varios problemas en C++ moderno:
- **Fugas de Memoria**: Es fácil olvidar llamar a `glDeleteBuffers`.
- **Seguridad de Tipos**: Un `GLuint` puede ser una textura, un buffer o un shader.
- **Verbosidad**: Configurar un VAO requiere múltiples llamadas a `glBindBuffer`, `glEnableVertexAttribArray`, `glVertexAttribPointer`, etc.

Nuestra implementación resuelve esto mediante el patrón **RAII (Resource Acquisition Is Initialization)**:
- Los recursos se crean en el constructor.
- Los recursos se liberan automáticamente en el destructor.
- Se prohíbe la copia (para evitar doble liberación) y se permite el movimiento (`std::move`).

## 2. Arquitectura de Clases

El sistema se encuentra en `src/graphics/rendering/buffer_objects.h`.

### 2.1. Clase Base: `Buffer`
Clase abstracta que maneja el ciclo de vida de un buffer genérico de OpenGL.
- **Responsabilidad**: `glGenBuffers` y `glDeleteBuffers`.
- **Tipos**: Soporta `GL_ARRAY_BUFFER` (Vértices) y `GL_ELEMENT_ARRAY_BUFFER` (Índices).
- **Uso**: `STATIC_DRAW` (Terreno, Modelos) vs `DYNAMIC_DRAW` (HUD, Instrumentos).

### 2.2. `VertexBuffer` (VBO)
Especialización para datos de vértices.
- **Método Principal**: `setData<T>(std::vector<T>)` sube datos a la GPU.
- **Método de Actualización**: `updateData<T>` permite modificar sub-secciones del buffer (usado en instrumentos del HUD que cambian cada frame).

### 2.3. `IndexBuffer` (IBO)
Especialización para índices de dibujo.
- **Funcionalidad Extra**: Almacena el conteo de índices (`count_`), necesario para la llamada `glDrawElements`.

### 2.4. `VertexArray` (VAO)
La clase principal que orquesta todo. Representa un objeto de estado que "recuerda" qué buffers usar y cómo interpretar sus datos.
- **Gestión de VBOs**: Mantiene `std::unique_ptr` a sus VBOs para asegurar que vivan tanto como el VAO.
- **Configuración de Atributos**: El método `addAttribute` simplifica drásticamente `glVertexAttribPointer`. Calcula automáticamente strides y offsets si es necesario.

## 3. Integración en el Simulador

Estas clases son fundamentales para todos los sistemas de renderizado del proyecto:

### 3.1. HUD (Head-Up Display)
Los instrumentos del HUD (Altímetro, Velocímetro, etc.) requieren actualizaciones constantes (60 FPS).
- **Implementación**: Usan `VertexBuffer` con `GL_DYNAMIC_DRAW`.
- **Optimización**: En lugar de recrear el buffer, usan `updateData` para modificar solo los valores que cambian (ej. la posición de la cinta de números).
- **Archivos**: `src/hud/instruments/*.cpp`

### 3.2. Sistema de Terreno (`ChunkedTerrain`)
El terreno infinito genera y destruye "chunks" dinámicamente mientras el avión vuela.
- **Beneficio Clave**: Gracias a los `std::unique_ptr<VertexArray>` en la estructura `Chunk`, cuando un chunk sale del rango y es eliminado del `std::vector`, la memoria de video se libera automáticamente. Esto previene fugas de memoria críticas en sesiones largas de vuelo.
- **Archivos**: `src/scene/chunked_terrain.cpp`

### 3.3. Renderizado de Misiones (`WaypointRenderer`)
Dibuja los marcadores de ruta en el mundo 3D.
- **Uso**: Utiliza un `VertexArray` simple para dibujar cilindros semitransparentes que representan los waypoints.
- **Archivos**: `src/mission/waypoint_renderer.cpp`

## 4. Ejemplo de Implementación Real

A continuación se muestra cómo se utiliza esta abstracción en el `WaypointRenderer` del simulador:

```cpp
// 1. Definición de datos (Vértices)
std::vector<float> vertices = { ... }; // x, y, z

// 2. Creación del VAO (El contenedor principal)
auto vao = std::make_unique<VertexArray>();

// 3. Creación del VBO (El buffer de datos)
auto vbo = std::make_unique<VertexBuffer>(BufferUsage::STATIC_DRAW);
vbo->setData(vertices);

// 4. Configuración del Layout (Cómo leer los datos)
// Atributo 0: Posición (3 floats)
vao->addVertexBuffer(std::move(vbo));
vao->addFloatAttribute(0, 3, 3 * sizeof(float), (void*)0);

// 5. Renderizado (En el bucle principal)
shader->use();
vao->bind();
glDrawArrays(GL_TRIANGLE_STRIP, 0, vertex_count);
vao->unbind();
```

## 5. Ventajas del Sistema

1. **Seguridad de Memoria**: Imposible olvidar liberar un buffer.
2. **Código Limpio**: Reduce 10-15 líneas de código OpenGL repetitivo a 3-4 líneas de C++ expresivo.
3. **Flexibilidad**: Soporta fácilmente cambios entre dibujo estático y dinámico.
4. **Depuración**: Centraliza las llamadas a OpenGL, facilitando la inserción de logs o chequeos de errores.

---
