#ifndef VERTEX_ARRAY_H
#define VERTEX_ARRAY_H

#include <glad/glad.h>
#include <vector>
#include <algorithm>

class VertexArray {
public:
    unsigned int ID;
    std::vector<unsigned int> enabled_attributes; // Para trackear atributos habilitados

    // Constructor genera un VAO
    VertexArray() {
        glGenVertexArrays(1, &ID);
    }

    // Destructor - deshabilita todos los atributos
    ~VertexArray() {
        unbind();
        // Opcional: deshabilitar atributos (OpenGL lo hace automáticamente al borrar el VAO)
        glDeleteVertexArrays(1, &ID);
    }

    // Bind el VAO
    void bind() const {
        glBindVertexArray(ID);
    }

    // Unbind el VAO
    void unbind() const {
        glBindVertexArray(0);
    }

    // Configurar y habilitar atributo de vértice (versión completa)
    void setVertexAttribute(unsigned int index, int size, GLenum type, 
                           GLboolean normalized, int stride, const void* pointer) {
        bind();
        glVertexAttribPointer(index, size, type, normalized, stride, pointer);
        glEnableVertexAttribArray(index);
        enabled_attributes.push_back(index); // Trackear atributo habilitado
    }

    // Versión simplificada para atributos comunes
    void addVertexAttribute(unsigned int index, int size, GLenum type = GL_FLOAT, 
                           bool normalized = false, int stride = 0, const void* pointer = 0) {
        setVertexAttribute(index, size, type, normalized ? GL_TRUE : GL_FALSE, 
                          stride, pointer);
    }

    // Deshabilitar un atributo específico (opcional)
    void disableAttribute(unsigned int index) {
        bind();
        glDisableVertexAttribArray(index);
        // Remover de la lista de atributos habilitados
        enabled_attributes.erase(
            std::remove(enabled_attributes.begin(), enabled_attributes.end(), index),
            enabled_attributes.end()
        );
    }

    // Deshabilitar todos los atributos (opcional)
    void disableAllAttributes() {
        bind();
        for (unsigned int attr : enabled_attributes) {
            glDisableVertexAttribArray(attr);
        }
        enabled_attributes.clear();
    }
};

#endif