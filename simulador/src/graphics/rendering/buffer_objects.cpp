#include "buffer_objects.h"
#include <iostream>

namespace Graphics {
    namespace Rendering {

        // === Implementación de Buffer ===

        Buffer::Buffer(BufferType type, BufferUsage usage)
            : buffer_id_(0), type_(type), usage_(usage), size_(0), bound_(false) {
            glGenBuffers(1, &buffer_id_);
        }

        Buffer::~Buffer() {
            if (buffer_id_ != 0) {
                glDeleteBuffers(1, &buffer_id_);
            }
        }

        Buffer::Buffer(Buffer&& other) noexcept
            : buffer_id_(other.buffer_id_), type_(other.type_), usage_(other.usage_),
              size_(other.size_), bound_(other.bound_) {
            other.buffer_id_ = 0;
            other.size_ = 0;
            other.bound_ = false;
        }

        Buffer& Buffer::operator=(Buffer&& other) noexcept {
            if (this != &other) {
                if (buffer_id_ != 0) {
                    glDeleteBuffers(1, &buffer_id_);
                }
                
                buffer_id_ = other.buffer_id_;
                type_ = other.type_;
                usage_ = other.usage_;
                size_ = other.size_;
                bound_ = other.bound_;
                
                other.buffer_id_ = 0;
                other.size_ = 0;
                other.bound_ = false;
            }
            return *this;
        }

        void Buffer::bind() {
            glBindBuffer(static_cast<GLenum>(type_), buffer_id_);
            bound_ = true;
        }

        void Buffer::unbind() {
            glBindBuffer(static_cast<GLenum>(type_), 0);
            bound_ = false;
        }

        // === Implementación de VertexBuffer ===

        VertexBuffer::VertexBuffer(BufferUsage usage)
            : Buffer(BufferType::VERTEX_BUFFER, usage) {
        }

        // === Implementación de IndexBuffer ===

        IndexBuffer::IndexBuffer(BufferUsage usage)
            : Buffer(BufferType::INDEX_BUFFER, usage), count_(0) {
        }

        // === Implementación de VertexArray ===

        VertexArray::VertexArray() : vao_id_(0), bound_(false) {
            glGenVertexArrays(1, &vao_id_);
        }

        VertexArray::~VertexArray() {
            if (vao_id_ != 0) {
                glDeleteVertexArrays(1, &vao_id_);
            }
        }

        VertexArray::VertexArray(VertexArray&& other) noexcept
            : vao_id_(other.vao_id_), 
              vertex_buffers_(std::move(other.vertex_buffers_)),
              index_buffer_(std::move(other.index_buffer_)),
              attributes_(std::move(other.attributes_)),
              bound_(other.bound_) {
            other.vao_id_ = 0;
            other.bound_ = false;
        }

        VertexArray& VertexArray::operator=(VertexArray&& other) noexcept {
            if (this != &other) {
                if (vao_id_ != 0) {
                    glDeleteVertexArrays(1, &vao_id_);
                }
                
                vao_id_ = other.vao_id_;
                vertex_buffers_ = std::move(other.vertex_buffers_);
                index_buffer_ = std::move(other.index_buffer_);
                attributes_ = std::move(other.attributes_);
                bound_ = other.bound_;
                
                other.vao_id_ = 0;
                other.bound_ = false;
            }
            return *this;
        }

        void VertexArray::bind() {
            glBindVertexArray(vao_id_);
            bound_ = true;
        }

        void VertexArray::unbind() {
            glBindVertexArray(0);
            bound_ = false;
        }

        void VertexArray::addVertexBuffer(std::unique_ptr<VertexBuffer> vb) {
            bind();
            vb->bind();
            vertex_buffers_.push_back(std::move(vb));
        }

        void VertexArray::setIndexBuffer(std::unique_ptr<IndexBuffer> ib) {
            bind();
            ib->bind();
            index_buffer_ = std::move(ib);
        }

        void VertexArray::addAttribute(GLuint index, GLint size, GLenum type,
                                     GLboolean normalized, GLsizei stride, const void* pointer) {
            bind();
            
            glEnableVertexAttribArray(index);
            glVertexAttribPointer(index, size, type, normalized, stride, pointer);
            
            attributes_.emplace_back(index, size, type, normalized, stride, pointer);
        }

        void VertexArray::addFloatAttribute(GLuint index, GLint size, GLsizei stride, const void* pointer) {
            addAttribute(index, size, GL_FLOAT, GL_FALSE, stride, pointer);
        }

        void VertexArray::addIntAttribute(GLuint index, GLint size, GLsizei stride, const void* pointer) {
            addAttribute(index, size, GL_INT, GL_FALSE, stride, pointer);
        }

    } // namespace Rendering
} // namespace Graphics