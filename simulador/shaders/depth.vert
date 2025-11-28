#version 330 core

/**
 * Vertex Shader para Shadow Pass
 * 
 * Renderiza la escena desde la perspectiva de la luz direccional
 * Solo necesitamos calcular la posición en clip space de la luz
 * 
 * Basado en técnica de Shadow Mapping:
 * - Transforma vértices al espacio de la luz
 * - OpenGL automáticamente escribe la profundidad al depth buffer
 */

layout (location = 0) in vec3 aPos;

uniform mat4 lightSpaceMatrix;  // Proyección * Vista de la luz
uniform mat4 model;

void main()
{
    // Transformar posición al espacio de la luz
    gl_Position = lightSpaceMatrix * model * vec4(aPos, 1.0);
}
