# Simulador de Vuelo OGL-Engine

![OpenGL](https://img.shields.io/badge/OpenGL-4.6-green.svg) ![C++](https://img.shields.io/badge/C++-17-blue.svg) ![Platform](https://img.shields.io/badge/Platform-Linux%20%7C%20Windows-lightgrey.svg)

Un simulador de vuelo avanzado desarrollado en C++ moderno y OpenGL 4.6. Este proyecto implementa un motor gráfico modular desde cero, integrando física de vuelo aerodinámica, generación de terreno procedural, sistemas de misiones y una interfaz de visualización frontal (HUD) vectorial completa.

## Características Principales

### Dinámica de Vuelo y Física
*   **Modelo Aerodinámico:** Simulación de fuerzas de sustentación, arrastre y empuje basadas en la física.
*   **Superficies de Control:** Control independiente de alerones (roll), elevadores (pitch) y timón (yaw).
*   **Sistema de Trim:** Ajuste fino de superficies de control para vuelo estable y nivelado.
*   **Gestión de Motor:** Control de aceleración con inercia simulada.

### Motor Gráfico y Renderizado
*   **Shadow Mapping:** Implementación de sombras dinámicas en tiempo real para el terreno y la aeronave.
*   **Terreno Procedural:** Sistema `ChunkedTerrain` basado en ruido Perlin para generación infinita de paisajes.
*   **Skybox:** Renderizado de entorno cúbico para inmersión atmosférica.
*   **Iluminación:** Sistema de iluminación direccional (Sol) con componentes ambientales y difusos.
*   **Efectos:** Niebla volumétrica (Fog) ajustable y modos de visualización (Wireframe, Texturas).

### Head-Up Display (HUD)
Instrumentación de vuelo completa renderizada vectorialmente:
*   **Pitch Ladder:** Escalera de cabeceo con horizonte artificial.
*   **Bank Angle:** Indicador de ángulo de alabeo.
*   **Altimeter:** Altímetro de cinta con lectura digital precisa.
*   **Speed Indicator:** Indicador de velocidad aérea (IAS).
*   **VSI:** Indicador de velocidad vertical (variómetro).
*   **Waypoint Indicator:** Guía visual hacia el siguiente objetivo de la misión.

### Sistema de Misiones
*   **Navegación por Waypoints:** Sistema de detección y seguimiento de puntos de ruta 3D.
*   **Misiones Definibles:** Estructura flexible para crear circuitos de navegación (ej. "Circuito de Navegación Extremo").
*   **Feedback Visual:** Indicadores en pantalla de distancia y dirección al objetivo.

### Sistema de Cámaras
*   **Primera Persona (Cockpit):** Vista desde la cabina.
*   **Tercera Persona:** Vista externa clásica con seguimiento suave.
*   **Cámara Cinemática:** Modo espectador inteligente que encuadra la acción dinámicamente con offsets ajustables.

---

## Arquitectura del Proyecto

El proyecto sigue una arquitectura modular estricta para separar responsabilidades:

```
simulador/
├── src/
│   ├── core/           # Contexto OpenGL y bucle principal
│   ├── graphics/       # Shaders, texturas, iluminación, skybox
│   ├── scene/          # Cámara, terreno, modelos 3D
│   ├── physics/        # Dinámica de vuelo y física
│   ├── hud/            # Instrumentos e interfaz de usuario
│   ├── mission/        # Lógica de misiones y waypoints
│   ├── input/          # Gestión de teclado, mouse y joystick
│   └── dlfdm/          # Data-Driven Flight Dynamics Model (Core físico)
├── include/            # Headers externos (stb_image, glad, etc.)
├── shaders/            # Código fuente GLSL (Vertex y Fragment shaders)
├── textures/           # Assets gráficos (avión, terreno, cielo)
└── docs/               # Documentación técnica detallada
```

---

## Requisitos y Dependencias

Para compilar y ejecutar el simulador, necesitas las siguientes librerías instaladas en tu sistema:

*   **Compilador C++:** Compatible con C++17 (GCC, Clang, MSVC).
*   **Make:** Para la construcción del proyecto.
*   **GLFW3:** Gestión de ventanas e input.
*   **GLM:** Matemáticas para gráficos (vectores, matrices).
*   **Assimp:** Carga de modelos 3D (.obj, .fbx).
*   **OpenGL Drivers:** Drivers de GPU actualizados.

**Instalación de dependencias en Ubuntu/Debian:**
```bash
sudo apt-get update
sudo apt-get install build-essential libglfw3-dev libglm-dev libassimp-dev mesa-common-dev
```

---

## Compilación e Instalación

1.  **Clonar el repositorio:**
    ```bash
    git clone <url-del-repo>
    cd computacion_grafica/simulador
    ```

2.  **Compilar el proyecto:**
    El proyecto utiliza un `Makefile` robusto. Simplemente ejecuta:
    ```bash
    make
    ```
    Esto generará el ejecutable en la carpeta `build/`.

3.  **Ejecutar:**
    ```bash
    make run
    ```
    O manualmente:
    ```bash
    ./build/OGL-Engine
    ```

4.  **Limpiar archivos temporales:**
    ```bash
    make clean
    ```

---

## Controles

El simulador soporta teclado y mouse:

### Controles de Vuelo (Física)
| Tecla | Acción | Descripción |
|-------|--------|-------------|
| **W** | Acelerar | Aumentar potencia del motor |
| **S** | Desacelerar | Disminuir potencia del motor |
| **↑ / ↓** | Elevador | Control de cabeceo (Pitch) |
| **← / →** | Alerones | Control de alabeo (Roll) |
| **A / D** | Timón | Control de guiñada (Yaw) |

### Sistema de Trim (Estabilidad)
| Tecla | Acción |
|-------|--------|
| **7** | Trim Elevador ARRIBA (Nariz arriba) |
| **8** | Trim Elevador ABAJO (Nariz abajo) |
| **9** | Resetear Trim (Vuelo nivelado) |

### Cámara y Visualización
| Tecla | Acción |
|-------|--------|
| **C** | Alternar cámara 1ª / 3ª persona |
| **V** | Alternar modo Cinemático / Clásico |
| **3-6** | Ajustes de cámara cinemática (Distancia, Offset, Altura) |

### Sistema y Debug
| Tecla | Acción |
|-------|--------|
| **G** | Modo Wireframe (Malla de alambre) |
| **T** | Activar/Desactivar Texturas |
| **F** | Activar/Desactivar Niebla |
| **1** | Imprimir controles en consola |
| **ESC** | Salir del simulador |

---

## Detalles Técnicos

### Implementación de Sombras
El sistema utiliza **Shadow Mapping** en dos pasadas:
1.  **Depth Pass:** Renderiza la escena desde la perspectiva de la luz solar en un framebuffer de profundidad.
2.  **Render Pass:** Utiliza el mapa de profundidad proyectado para determinar si un fragmento está en sombra, aplicando el cálculo en el fragment shader.

### Generación de Terreno
El terreno se genera dinámicamente utilizando **Chunked LOD** (Level of Detail implícito por distancia de renderizado) y **Ruido Perlin**. Esto permite:
*   Terrenos infinitos o muy extensos sin cargar modelos pesados.
*   Variación de altura realista.
*   Texturizado basado en coordenadas UV globales.

### HUD Vectorial
Los instrumentos no son texturas estáticas, sino geometría dibujada en tiempo real mediante shaders específicos (`vertex_hud.glsl`, `fragment_hud.glsl`). Esto garantiza nitidez infinita a cualquier resolución y actualizaciones fluidas a 60+ FPS.

---

## Documentación Adicional

Para detalles profundos sobre implementaciones específicas, consulta la carpeta `docs/`:
*   `BUFFER_OBJECTS_DOCUMENTATION.md`: Gestión de memoria OpenGL (VAO, VBO).
*   `CAMERA_CINEMATIC.md`: Lógica matemática de la cámara de seguimiento.
*   `FOG_IMPLEMENTATION.md`: Implementación de niebla volumétrica y exponencial.
*   `SHADOW_MAPPING_IMPLEMENTATION.md`: Teoría y código del sistema de sombras.
*   `TRANSFORMATIONS_DOCUMENTATION.md`: Documentación sobre transformaciones geométricas y matrices.

---

## Licencia

Este proyecto es de uso académico y educativo.
Desarrollado por Alejo Gazagne y Gabriel Guillaumet.
