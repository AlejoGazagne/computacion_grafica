# Documentación de Transformaciones y Matrices

Este documento detalla cómo se utilizan las transformaciones matemáticas (matrices) en las diferentes partes del simulador de vuelo. El proyecto utiliza la librería **GLM (OpenGL Mathematics)** para todas las operaciones vectoriales y matriciales.

## 1. Sistema de Escena (Scene System)

El sistema de escena maneja la representación 3D del mundo, incluyendo la cámara, el terreno y los modelos.

### 1.1. Cámara (`src/scene/camera.cpp`)

La cámara es responsable de generar las matrices de Vista y Proyección que transforman las coordenadas del mundo a coordenadas de pantalla.

*   **Matriz de Vista (View Matrix):**
    *   **Función:** `glm::lookAt(position, target, up)`
    *   **Uso:** Transforma coordenadas de Espacio de Mundo (World Space) a Espacio de Vista (View/Eye Space).
    *   **Modos:**
        *   *Primera Persona:* La posición es la del avión.
        *   *Tercera Persona:* La posición se calcula restando el vector `front` a la posición del objetivo.
        *   *Cinemática:* La posición se calcula con offsets laterales y de altura relativos a la orientación del avión.

*   **Matriz de Proyección (Projection Matrix):**
    *   **Función:** `glm::perspective(fov, aspect_ratio, near_plane, far_plane)`
    *   **Uso:** Transforma coordenadas de Espacio de Vista a Espacio de Recorte (Clip Space).
    *   **Detalles:** Utiliza una proyección en perspectiva para simular la profundidad. El FOV (Campo de Visión) es dinámico para efectos de zoom.

### 1.2. Modelos 3D (`src/scene/model.h`)

Cada objeto 3D (como el avión) tiene su propia transformación de modelo, encapsulada en la estructura `Transform` definida en el header.

*   **Matriz de Modelo (Model Matrix):**
    *   **Ubicación:** Método `Transform::getMatrix()` en `src/scene/model.h`.
    *   **Construcción:** `Translation * Rotation * Scale`
    *   **Funciones:**
        *   `glm::translate(identity, position)`: Mueve el objeto.
        *   `glm::rotate(matrix, angle, axis)`: Rota el objeto (Pitch, Yaw, Roll).
        *   `glm::scale(matrix, scale_factor)`: Escala el objeto.
    *   **Uso:** Transforma vértices de Espacio Local (Local Space) a Espacio de Mundo (World Space). `model.cpp` simplemente invoca este método.

### 1.3. Terreno (`src/scene/chunked_terrain.cpp`)

El terreno se divide en "chunks" (fragmentos), pero a diferencia de los modelos tradicionales, **no utiliza una matriz de modelo para posicionar cada chunk**.

*   **Matriz de Modelo:**
    *   Se utiliza la matriz **Identidad** (`glm::mat4(1.0f)`).
*   **Posicionamiento de Vértices (World Space):**
    *   Las coordenadas de los vértices se calculan directamente en **Espacio de Mundo** (World Space) durante la construcción de la malla en la CPU (`buildChunkMesh`).
    *   Se calcula la posición absoluta `pos_x` y `pos_z` sumando el origen del chunk (`origin_x`, `origin_z`) a las coordenadas locales.
    *   Esto evita tener que recalcular matrices de transformación por cada chunk en cada frame, optimizando el renderizado de geometría estática.

### 1.4. Skybox (`src/graphics/skybox/skybox.cpp`)

El cielo debe parecer infinitamente lejano, por lo que no debe moverse con la cámara, solo rotar.

*   **Matriz de Vista Modificada:**
    *   **Operación:** `glm::mat4(glm::mat3(view))`
    *   **Ubicación:** Método `Skybox::render`.
    *   **Explicación:** Se convierte la matriz de vista 4x4 a 3x3. Esto elimina la cuarta columna que contiene la información de traslación, manteniendo solo la rotación. Luego se vuelve a convertir a 4x4. Esto hace que el skybox siempre esté centrado en el origen de la cámara (0,0,0), creando la ilusión de distancia infinita.

---

## 2. Sistema de Iluminación y Sombras (Lighting & Shadows)

Para generar sombras dinámicas (Shadow Mapping), se renderiza la escena desde la perspectiva de la luz.

### 2.1. Luz Direccional (`src/graphics/lighting/light.h`)

*   **Matriz de Espacio de Luz (Light Space Matrix):**
    *   **Ubicación:** Método `DirectionalLight::getLightSpaceMatrix`.
    *   **Fórmula:** `LightProjection * LightView`
    *   **Light View:** `glm::lookAt(light_pos, target_pos, up)`
        *   Simula una "cámara" en la posición de la luz mirando hacia el jugador.
    *   **Light Projection:** `glm::ortho(left, right, bottom, top, near, far)`
        *   **Importante:** Se usa proyección **Ortográfica** (`glm::ortho`) en lugar de perspectiva. Esto es crucial para luces direccionales (como el sol) porque sus rayos son paralelos, lo que significa que las sombras no deben distorsionarse con la distancia.

---

## 3. Sistema HUD (Head-Up Display)

El HUD se renderiza en 2D sobre la escena 3D.

### 3.1. Instrumentos (`src/hud/instruments/*.cpp`)

*   **Matriz de Proyección:**
    *   **Ubicación:** Método `render` de cada instrumento (ej. `Altimeter::render`, `VerticalSpeedIndicator::render`).
    *   **Función:** `glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f)`
    *   **Uso:** Mapea directamente a Coordenadas de Dispositivo Normalizadas (NDC). La pantalla va de -1 a 1 en X e Y. Esto facilita el posicionamiento 2D sin preocuparse por la perspectiva.

*   **Matriz de Modelo:**
    *   Generalmente es la identidad (`glm::mat4(1.0f)`).
    *   **Nota de Implementación:** En lugar de usar matrices `glm::translate` o `glm::rotate` para mover las agujas o cintas, los instrumentos calculan las posiciones de sus vértices dinámicamente en la CPU (C++) basándose en los datos de vuelo, y luego envían esos vértices ya transformados a la GPU.

---

## 4. Sistema de Misiones (Mission System)

### 4.1. Waypoints (`src/mission/waypoint_renderer.cpp`)

Los marcadores de misión flotantes en el mundo 3D.

*   **Matriz de Modelo:**
    *   **Ubicación:** Método `WaypointRenderer::drawWaypoint`.
    *   **Operación:** `glm::translate(glm::mat4(1.0f), position)`
    *   **Uso:** Mueve el cilindro genérico (definido en el origen) a la posición del waypoint en el mundo.
*   **Matrices de Cámara:** Utilizan las mismas matrices `View` y `Projection` de la cámara principal para que aparezcan correctamente integrados en el mundo 3D.

---