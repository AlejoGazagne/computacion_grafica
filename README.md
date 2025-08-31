# Proyecto de Computación Gráfica

Este repositorio contiene implementaciones de algoritmos clásicos de computación gráfica y un template para proyectos con OpenGL.

## Tecnologías utilizadas

### Algoritmos Clásicos
- **C**: Los algoritmos como Bresenham, scan-line y pmc están implementados en C, permitiendo el manejo eficiente de gráficos a bajo nivel.

### OpenGL
- **OpenGL 4.6**: El template utiliza OpenGL para renderizado gráfico acelerado por hardware.
- **GLAD**: Se emplea GLAD como cargador de funciones de OpenGL, facilitando la compatibilidad y el acceso a las últimas versiones de la API.
- **GLSL**: Los shaders están escritos en GLSL (OpenGL Shading Language), permitiendo la programación de la GPU para efectos visuales avanzados.

### Estructura del Proyecto
- **C++**: El núcleo del template está desarrollado en C++, aprovechando la orientación a objetos para la gestión de buffers, shaders y arrays de vértices.
- **Makefile**: El sistema de construcción utiliza Makefile para compilar y enlazar los archivos fuente de manera eficiente.
- **Carpetas organizadas**: Separación clara entre código fuente (`src/`), shaders (`shaders/`), cabeceras (`include/`), y binarios (`build/`).

## Cómo compilar

Dentro de la carpeta `ogl-project-template`, ejecuta:
```bash
make
```
Esto generará el ejecutable principal en la carpeta `build/`.

## Créditos

- GLAD: https://glad.dav1d.de/
- OpenGL: https://www.opengl.org/
- Algoritmos clásicos: Implementación propia en C.
