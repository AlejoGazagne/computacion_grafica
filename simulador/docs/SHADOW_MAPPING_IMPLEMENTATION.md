# Sistema de Shadow Mapping

Este documento detalla la arquitectura e implementación del sistema de sombras dinámicas en tiempo real para el simulador de vuelo. El sistema utiliza la técnica de **Shadow Mapping** con proyección ortográfica para simular la luz solar direccional.

## 1. Propósito y Motivación

En un simulador de vuelo, las sombras son críticas para la percepción espacial:
- **Percepción de Altitud**: La sombra del avión sobre el terreno ayuda al piloto a estimar su altura durante el aterrizaje y vuelo rasante.
- **Realismo Visual**: Conecta los objetos (avión, terreno) en una escena coherente.
- **Orientación**: La dirección de las sombras refuerza la posición del sol en el skybox.

## 2. Arquitectura de Clases

El núcleo del sistema es la clase `ShadowMap`, ubicada en `src/graphics/rendering/shadow_map.h`. Esta clase encapsula la complejidad de los Framebuffer Objects (FBO) de OpenGL.

### 2.1. Clase `ShadowMap`
Responsable de gestionar la memoria de video donde se almacena el mapa de profundidad.

- **Constructor**: `ShadowMap(width, height)`
  - Inicializa un FBO dedicado.
  - Crea una textura 2D de profundidad (`GL_DEPTH_COMPONENT`).
  - Configura parámetros de textura críticos:
    - `GL_NEAREST`: Filtrado sin interpolación (necesario para la comparación de profundidad precisa).
    - `GL_CLAMP_TO_BORDER`: Evita que se proyecten sombras fuera del área cubierta por el mapa (borde blanco).

- **Métodos Principales**:
  - `bindForWriting()`: Configura el viewport y activa el FBO para el primer pase (captura de sombras). Limpia el buffer de profundidad automáticamente.
  - `bindForReading(texture_unit)`: Activa la textura de profundidad en una unidad específica para ser leída por los shaders durante el segundo pase.
  - `unbind()`: Restaura el framebuffer por defecto (pantalla).

## 3. Integración en el Motor Gráfico

La clase `GraphicsEngine` (`src/core/graphics_engine.h`) orquesta el proceso de renderizado en dos etapas.

### 3.1. Primer Pase: Captura de Sombras (`renderShadowPass`)
1. **Cálculo de Matriz de Luz**: Se calcula una matriz de vista/proyección ortográfica centrada en el avión/cámara. Esto asegura que el mapa de sombras siempre cubra el área visible más relevante ("Shadow Following").
2. **Renderizado**: Se activa el `ShadowMap` y se renderiza la escena (Terreno y Avión) usando un shader simplificado (`depth.vert`/`depth.frag`) que solo escribe información de profundidad.
   - *Optimización*: El avión usa un método `renderDepthOnly` para evitar cambios de estado innecesarios.

### 3.2. Segundo Pase: Renderizado de Escena (`render`)
1. **Configuración**: Se restaura el viewport de la ventana y se activa el shader principal (`basic_3d`).
2. **Uniforms**: Se envían al shader:
   - `lightSpaceMatrix`: La misma matriz usada en el primer pase.
   - `shadowMap`: La textura de profundidad (vinculada en la unidad 5).
3. **Iluminación**: El shader calcula si cada píxel está en sombra comparando su profundidad con la del mapa.

## 4. Detalles Técnicos de Implementación

### 4.1. Configuración del FBO
- **Resolución**: 4096 x 4096 píxeles. Esta alta resolución es necesaria para cubrir grandes áreas de terreno sin pixelación excesiva ("aliasing").
- **Formato**: Solo profundidad (`GL_DEPTH_ATTACHMENT`), sin buffer de color (`glDrawBuffer(GL_NONE)`).

### 4.2. Técnicas de Suavizado (En Shaders)
Para evitar bordes duros y artefactos visuales, implementamos en `fragment_3d.glsl`:
- **PCF (Percentage-Closer Filtering)**: Muestrea una cuadrícula de 3x3 texels alrededor del punto actual y promedia el resultado. Esto suaviza los bordes de las sombras.
- **Bias Adaptativo**: `max(0.05 * (1.0 - dot(normal, lightDir)), 0.005)`. Ajusta el margen de error de profundidad basándose en el ángulo de la superficie para prevenir "Shadow Acne" (patrones rayados en la superficie).

## 5. Ejemplo de Flujo de Renderizado

```cpp
// En GraphicsEngine::render()

// --- PASO 1: SHADOW PASS ---
// Calcular matriz de luz centrada en el jugador
glm::mat4 lightSpaceMatrix = sunlight->getLightSpaceMatrix(camera_pos);

shadow_map_->bindForWriting(); // Activa FBO, limpia depth
depth_shader_->use();
depth_shader_->setMat4("lightSpaceMatrix", lightSpaceMatrix);

// Renderizar solo geometría
chunked_terrain_->draw();
aircraft_model_->renderDepthOnly(depth_shader_);

shadow_map_->unbind(); // Volver al framebuffer de pantalla

// --- PASO 2: LIGHTING PASS ---
glViewport(0, 0, window_width, window_height);
glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

main_shader_->use();
// Pasar matriz y textura de sombras
main_shader_->setMat4("lightSpaceMatrix", lightSpaceMatrix);
shadow_map_->bindForReading(GL_TEXTURE0 + 5);
main_shader_->setInt("shadowMap", 5);

// Renderizar escena con iluminación y sombras
renderScene();
```

## 6. Ventajas del Sistema

1. **Modularidad**: La clase `ShadowMap` es independiente y reutilizable.
2. **Calidad Visual**: El uso de PCF y alta resolución (4K) produce sombras suaves y definidas.
3. **Rendimiento**: Al usar un shader de profundidad extremadamente simple en el primer pase, el impacto en el rendimiento es mínimo.
4. **Robustez**: La técnica de "Shadow Following" permite sombras detalladas cerca del jugador sin requerir mapas de sombras infinitamente grandes.

---