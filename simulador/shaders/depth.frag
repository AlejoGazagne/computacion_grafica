#version 330 core

/**
 * Fragment Shader para Shadow Pass
 * 
 * No necesita hacer nada - OpenGL automáticamente escribe gl_FragDepth
 * al depth buffer attachado al FBO.
 * 
 * Este shader está vacío intencionalmente porque solo nos interesa
 * la profundidad que se escribe automáticamente.
 */

void main()
{
    // OpenGL escribe automáticamente gl_FragDepth
    // No necesitamos escribir ningún color (el FBO no tiene color attachment)
}
