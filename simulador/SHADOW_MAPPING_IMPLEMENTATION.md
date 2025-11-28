# Implementación de Shadow Mapping en el Simulador de Vuelo

## Resumen Ejecutivo

Se implementó exitosamente un sistema de **Shadow Mapping** basado en la técnica estándar de OpenGL para renderizado de sombras en tiempo real. El sistema proyecta sombras realistas del avión sobre el terreno utilizando la luz solar direccional existente.

---

## 1. Fundamentos Teóricos

### 1.1 Shadow Mapping

Shadow Mapping es una técnica de renderizado en dos pasadas que determina qué píxeles están en sombra:

1. **Primera pasada (Shadow Pass)**: Renderiza la escena desde la perspectiva de la luz, guardando solo la profundidad en un texture (depth map/shadow map).

2. **Segunda pasada (Render Pass)**: Renderiza la escena normalmente, pero para cada píxel:
   - Transforma su posición al espacio de la luz
   - Compara su profundidad con la del shadow map
   - Si la profundidad actual es mayor → el píxel está en sombra

### 1.2 Técnicas Implementadas

- **Proyección Ortográfica**: Para luces direccionales (sol), se usa proyección ortográfica ya que los rayos de luz son paralelos.
- **PCF (Percentage Closer Filtering)**: Suaviza los bordes de las sombras muestreando 9 texels vecinos (kernel 3x3).
- **Depth Bias**: Ajuste adaptativo para evitar "shadow acne" (patrones de bandas).
- **Border Clamping**: Fuera del frustum de luz = sin sombra.

---

## 2. Componentes Implementados

### 2.1 Clase `ShadowMap` (shadow_map.h)

**Ubicación**: `src/graphics/rendering/shadow_map.h`

**Propósito**: Encapsula el manejo del Framebuffer Object (FBO) y la textura de profundidad.

**Características**:

```cpp
- Resolución: 4096x4096 (alta calidad)
- Formato: GL_DEPTH_COMPONENT (solo profundidad, sin color)
- Filtrado: GL_NEAREST (sin interpolación de profundidad)
- Wrap mode: GL_CLAMP_TO_BORDER (borde blanco = sin sombra)
```

**Métodos principales**:

- `initialize()`: Crea FBO y textura de profundidad
- `bindForWriting()`: Activa FBO para shadow pass
- `bindForReading(texture_unit)`: Activa textura para render pass
- `unbind()`: Restaura framebuffer default

**Referencias teóricas**:

- OpenGL Framebuffer Objects (FBO)
- Depth testing y depth buffer
- Texture attachments

---

### 2.2 Shaders de Shadow Pass

#### depth.vert (Vertex Shader)

**Ubicación**: `shaders/depth.vert`

**Función**: Transforma vértices al espacio de la luz (clip space).

```glsl
gl_Position = lightSpaceMatrix * model * vec4(aPos, 1.0);
```

- `lightSpaceMatrix`: Proyección ortográfica × Vista de la luz
- `model`: Transformación del objeto
- Resultado: Posición en clip space de la luz

#### depth.frag (Fragment Shader)

**Ubicación**: `shaders/depth.frag`

**Función**: Vacío intencionalmente - OpenGL escribe automáticamente `gl_FragDepth`.

**Nota**: No necesita escribir color porque el FBO no tiene color attachment, solo depth attachment.

---

### 2.3 Modificaciones en DirectionalLight

**Ubicación**: `src/graphics/lighting/light.h`

**Nuevos parámetros**:

```cpp
float shadow_frustum_size_;   // Tamaño del volumen ortográfico (100m)
float shadow_near_plane_;     // Plano cercano (1m)
float shadow_far_plane_;      // Plano lejano (300m)
```

**Método clave**: `getLightSpaceMatrix(target_position)`

**Algoritmo**:

1. Calcula posición de la luz basada en su dirección y el objetivo (avión)
2. Crea matriz de vista usando `glm::lookAt()` desde la luz
3. Crea matriz de proyección ortográfica usando `glm::ortho()`
4. Retorna: `lightSpaceMatrix = Projection × View`

**Fundamento matemático**:

```
lightSpaceMatrix transforma de World Space → Light Clip Space
Esto permite comparar profundidades en el mismo sistema de coordenadas
```

---

### 2.4 Modificaciones en Shaders Principales

#### vertex_3d.glsl

**Cambios**:

```glsl
out vec4 FragPosLightSpace;  // Nueva variable de salida
uniform mat4 lightSpaceMatrix;

FragPosLightSpace = lightSpaceMatrix * vec4(FragPos, 1.0);
```

Calcula la posición del vértice en espacio de luz para usar en fragment shader.

#### fragment_3d.glsl

**Nuevos elementos**:

1. **Input adicional**:

```glsl
in vec4 FragPosLightSpace;
uniform sampler2D shadowMap;
```

2. **Función `ShadowCalculation()`**:

**Pasos del algoritmo**:

```glsl
// 1. Transformar de clip space [-1,1] a texture space [0,1]
vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
projCoords = projCoords * 0.5 + 0.5;

// 2. Obtener profundidad actual del fragmento
float currentDepth = projCoords.z;

// 3. Calcular bias adaptativo (previene shadow acne)
float bias = max(0.005 * (1.0 - dot(normal, lightDir)), 0.001);

// 4. PCF (Percentage Closer Filtering) - 3x3 kernel
float shadow = 0.0;
vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
for(int x = -1; x <= 1; ++x) {
    for(int y = -1; y <= 1; ++y) {
        float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
        shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
    }
}
shadow /= 9.0;  // Promedio

return shadow;  // 0.0 = sin sombra, 1.0 = sombra completa
```

3. **Integración en `CalcDirLight()`**:

```glsl
float shadow = ShadowCalculation(FragPosLightSpace, normal, lightDir);
return ambient + (1.0 - shadow) * diffuse;
```

**Nota importante**: La luz ambiental NO se ve afectada por sombras (realismo físico básico).

---

### 2.5 Integración en GraphicsEngine

**Ubicación**: `src/core/graphics_engine.h`

**Nuevos miembros**:

```cpp
std::unique_ptr<Graphics::Rendering::ShadowMap> shadow_map_;
Graphics::Shaders::Shader* depth_shader_;
```

**Método nuevo**: `renderShadowPass()`

**Flujo del método**:

1. Obtener luz solar del LightManager
2. Calcular `lightSpaceMatrix` siguiendo al avión
3. Activar FBO del shadow map
4. Usar shader de profundidad
5. Renderizar geometría (terreno + avión en 3ra persona)
6. Desactivar FBO

**Modificación del `render()` principal**:

```cpp
void render() {
    // ========== SHADOW PASS ==========
    renderShadowPass();

    // ========== RENDER PASS ==========
    // Restaurar viewport
    glViewport(0, 0, width, height);

    // Renderizado normal...
    shader->setMat4("lightSpaceMatrix", lightSpaceMatrix);
    shadow_map_->bindForReading(GL_TEXTURE0 + 5);
    shader->setInt("shadowMap", 5);

    // Renderizar escena...
}
```

---

## 3. Flujo Completo del Sistema

### 3.1 Inicialización

```
initializeGraphicsSystems()
  ↓
Cargar shader "depth" (depth.vert + depth.frag)
  ↓
Crear ShadowMap(4096, 4096)
  ↓
Inicializar FBO y textura de profundidad
```

### 3.2 Cada Frame

#### Shadow Pass (Primera pasada)

```
renderShadowPass()
  ↓
Calcular lightSpaceMatrix (seguir al avión)
  ↓
shadow_map_->bindForWriting()
  ↓
Configurar depth_shader con lightSpaceMatrix
  ↓
Renderizar terreno y avión
  ↓
OpenGL escribe automáticamente profundidad a depth texture
  ↓
shadow_map_->unbind()
```

#### Render Pass (Segunda pasada)

```
render()
  ↓
Restaurar viewport a tamaño de ventana
  ↓
Limpiar buffers (color + depth)
  ↓
Configurar shader normal con:
  - lightSpaceMatrix
  - shadowMap (texture unit 5)
  ↓
Renderizar escena completa
  ↓
Para cada píxel:
  - vertex shader: calcular FragPosLightSpace
  - fragment shader: ShadowCalculation()
    → comparar profundidad actual vs shadow map
    → aplicar PCF
    → retornar factor de sombra
  - aplicar factor a diffuse (no a ambient)
```

---

## 4. Parámetros de Configuración

### 4.1 Resolución del Shadow Map

```cpp
4096 × 4096 píxeles
```

**Justificación**: Alta resolución para sombras nítidas en terreno extenso.

### 4.2 Frustum de Luz (Proyección Ortográfica)

```cpp
shadow_frustum_size_ = 100.0f;   // Tamaño del cubo: 100m × 100m
shadow_near_plane_ = 1.0f;       // Plano cercano
shadow_far_plane_ = 300.0f;      // Plano lejano
```

### 4.3 PCF (Suavizado)

```glsl
Kernel: 3×3 (9 muestras)
```

### 4.4 Depth Bias

```glsl
bias = max(0.005 * (1.0 - dot(normal, lightDir)), 0.001);
```

**Adaptativo**: Mayor bias en superficies oblicuas a la luz.

---

## 5. Fundamentos Teóricos Aplicados

### 5.1 Transformaciones de Coordenadas

```
World Space
  ↓ (View Matrix)
Camera Space
  ↓ (Projection Matrix)
Clip Space [-1, 1]
  ↓ (Perspective Division)
NDC [-1, 1]
  ↓ (Viewport Transform)
Screen Space

Para Shadow Mapping:
World Space
  ↓ (Light View Matrix)
Light Camera Space
  ↓ (Light Projection Matrix)
Light Clip Space
  ↓ (Perspective Division)
Light NDC [-1, 1]
  ↓ (* 0.5 + 0.5)
Texture Space [0, 1]
```

### 5.2 Depth Testing

- OpenGL escribe automáticamente `gl_FragDepth` durante rasterización
- Depth test: compara profundidad actual vs profundidad en buffer
- En shadow pass: profundidad se guarda en textura en lugar de buffer estándar

### 5.3 Framebuffer Objects (FBO)

- Permite renderizar a texturas en lugar de pantalla
- Nuestro FBO: solo depth attachment, sin color
- `glDrawBuffer(GL_NONE)` y `glReadBuffer(GL_NONE)` → no usar color buffer

### 5.4 Texture Sampling

- `texture(shadowMap, coords)`: obtiene profundidad almacenada
- PCF: muestrea texels vecinos y promedia resultados
- Suaviza transiciones entre luz y sombra

---

## 6. Problemas Comunes y Soluciones

### 6.1 Shadow Acne (Patrones de Bandas)

**Causa**: Resolución finita del shadow map + auto-sombreado.
**Solución**: Depth bias adaptativo basado en ángulo de incidencia.

### 6.2 Peter Panning (Sombras Flotantes)

**Causa**: Bias demasiado grande.
**Solución**: Ajustar bias mínimo y máximo cuidadosamente.

### 6.3 Aliasing en Bordes

**Causa**: Resolución del shadow map y muestreo puntual.
**Solución**: PCF con kernel 3×3.

### 6.4 Sombras Fuera del Frustum

**Causa**: Geometría fuera del volumen de visión de la luz.
**Solución**: `GL_CLAMP_TO_BORDER` con borde blanco.

---

## 7. Referencias Teóricas

### 7.1 Libros Consultados

- **OpenGL 4.0 Shading Language Cookbook** - David Wolff
  - Capítulo: Shadow Mapping Techniques
- **Computer Graphics with OpenGL (4th ed.)** - Hearn, Baker & Carithers
  - Sección: Depth Testing and Buffers
  - Sección: Texture Mapping

### 7.2 Conceptos de OpenGL Aplicados

1. **Framebuffer Objects (FBO)**
2. **Depth Testing y Depth Buffer**
3. **Texture Attachments**
4. **Orthographic Projection**
5. **Coordinate Systems y Transformaciones**
6. **Multi-pass Rendering**
7. **Texture Sampling y Filtering**

### 7.3 Técnicas Gráficas

1. **Shadow Mapping (Básico)**
2. **Percentage Closer Filtering (PCF)**
3. **Adaptive Depth Bias**
4. **Two-pass Rendering**

---

## 8. Resultados y Validación

### 8.1 Funcionalidad Confirmada

✅ Sombras del avión proyectadas sobre el terreno
✅ Sombras se actualizan en tiempo real según posición del sol
✅ Bordes suavizados con PCF
✅ Sin shadow acne visible
✅ Rendimiento estable (dos pasadas de renderizado)

### 8.2 Características Visuales

- Sombras realistas con bordes suaves
- Intensidad correcta (ambient no afectado, diffuse sí)
- Cobertura adecuada del área de juego
- Sin artefactos visuales notables

---

## 9. Trabajo Futuro (Opcional)

### 9.1 Mejoras Posibles

- **Cascaded Shadow Maps (CSM)**: Para terrenos muy grandes
- **Soft Shadows**: PCF con kernels más grandes (5×5, 7×7)
- **Exponential Shadow Maps (ESM)**: Alternativa a depth comparison
- **Variance Shadow Maps (VSM)**: Reduce aliasing adicional

### 9.2 Optimizaciones

- **Culling selectivo** en shadow pass
- **Resolución adaptativa** del shadow map
- **Shadow map caching** para objetos estáticos

---

## 10. Conclusión

Se implementó exitosamente un sistema completo de Shadow Mapping siguiendo los fundamentos teóricos de OpenGL vistos en la materia. El sistema:

1. **Fundamentado teóricamente**: Basado en técnicas estándar documentadas
2. **Modular y extensible**: Clase ShadowMap reutilizable
3. **Eficiente**: Solo dos pasadas de renderizado
4. **Robusto**: Maneja casos edge (fuera de frustum, bias, etc.)
5. **Funcional**: Produce sombras realistas en tiempo real

La implementación demuestra comprensión profunda de:

- Renderizado multi-pasada
- Framebuffer Objects
- Transformaciones de coordenadas
- Depth testing
- Texture sampling y filtering
- Técnicas de suavizado (PCF)

---

**Fecha de implementación**: 27 de noviembre de 2025
**Sistema**: Flight Simulator - OpenGL 3.3 Core Profile
**Lenguaje**: C++17 + GLSL 3.30
