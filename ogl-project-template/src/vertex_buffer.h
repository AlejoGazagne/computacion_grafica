#ifndef VERTEX_BUFFER_H
#define VERTEX_BUFFER_H

#include <glad/glad.h>

class VertexBuffer {
public:
    unsigned int ID;

    // Constructor que genera un VBO y lo llena con datos
    VertexBuffer(const void* data, unsigned int size, GLenum usage = GL_STATIC_DRAW) {
        glGenBuffers(1, &ID);
        glBindBuffer(GL_ARRAY_BUFFER, ID);
        glBufferData(GL_ARRAY_BUFFER, size, data, usage);
    }

    // Destructor
    ~VertexBuffer() {
        glDeleteBuffers(1, &ID);
    }

    // Bind el VBO
    void bind() const {
        glBindBuffer(GL_ARRAY_BUFFER, ID);
    }

    // Unbind el VBO
    void unbind() const {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    // Actualizar datos del buffer (opcional)
    void updateData(const void* data, unsigned int size, GLenum usage = GL_STATIC_DRAW) {
        bind();
        glBufferData(GL_ARRAY_BUFFER, size, data, usage);
    }
};

#endif