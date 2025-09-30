#ifndef BUFFER_OBJECTS_H
#define BUFFER_OBJECTS_H

extern "C" {
    #include <glad/glad.h>
}

#include <vector>
#include <memory>

namespace Graphics {
    namespace Rendering {

        // Enum para tipos de buffer
        enum class BufferType {
            VERTEX_BUFFER = GL_ARRAY_BUFFER,
            INDEX_BUFFER = GL_ELEMENT_ARRAY_BUFFER,
            UNIFORM_BUFFER = GL_UNIFORM_BUFFER
        };

        // Enum para uso de buffer
        enum class BufferUsage {
            STATIC_DRAW = GL_STATIC_DRAW,
            DYNAMIC_DRAW = GL_DYNAMIC_DRAW,
            STREAM_DRAW = GL_STREAM_DRAW
        };

        // Buffer genérico
        class Buffer {
        private:
            GLuint buffer_id_;
            BufferType type_;
            BufferUsage usage_;
            size_t size_;
            bool bound_;

        public:
            Buffer(BufferType type = BufferType::VERTEX_BUFFER, 
                   BufferUsage usage = BufferUsage::STATIC_DRAW);
            ~Buffer();

            // No permitir copia
            Buffer(const Buffer&) = delete;
            Buffer& operator=(const Buffer&) = delete;

            // Permitir movimiento
            Buffer(Buffer&& other) noexcept;
            Buffer& operator=(Buffer&& other) noexcept;

            void bind();
            void unbind();
            
            template<typename T>
            void setData(const std::vector<T>& data);
            
            template<typename T>
            void setData(const T* data, size_t count);
            
            template<typename T>
            void updateData(const std::vector<T>& data, size_t offset = 0);

            GLuint getId() const { return buffer_id_; }
            BufferType getType() const { return type_; }
            size_t getSize() const { return size_; }
            bool isBound() const { return bound_; }
        };

        // Vertex Buffer Object especializado
        class VertexBuffer : public Buffer {
        public:
            VertexBuffer(BufferUsage usage = BufferUsage::STATIC_DRAW);
            ~VertexBuffer() = default;
        };

        // Index Buffer Object especializado
        class IndexBuffer : public Buffer {
        private:
            size_t count_;

        public:
            IndexBuffer(BufferUsage usage = BufferUsage::STATIC_DRAW);
            ~IndexBuffer() = default;

            template<typename T>
            void setIndices(const std::vector<T>& indices);

            size_t getCount() const { return count_; }
        };

        // Estructura para atributos de vértice
        struct VertexAttribute {
            GLuint index;
            GLint size;
            GLenum type;
            GLboolean normalized;
            GLsizei stride;
            const void* pointer;

            VertexAttribute(GLuint idx, GLint sz, GLenum tp, GLboolean norm, GLsizei str, const void* ptr)
                : index(idx), size(sz), type(tp), normalized(norm), stride(str), pointer(ptr) {}
        };

        // Vertex Array Object
        class VertexArray {
        private:
            GLuint vao_id_;
            std::vector<std::unique_ptr<VertexBuffer>> vertex_buffers_;
            std::unique_ptr<IndexBuffer> index_buffer_;
            std::vector<VertexAttribute> attributes_;
            bool bound_;

        public:
            VertexArray();
            ~VertexArray();

            // No permitir copia
            VertexArray(const VertexArray&) = delete;
            VertexArray& operator=(const VertexArray&) = delete;

            // Permitir movimiento
            VertexArray(VertexArray&& other) noexcept;
            VertexArray& operator=(VertexArray&& other) noexcept;

            void bind();
            void unbind();

            // Agregar vertex buffer y configurar atributos
            void addVertexBuffer(std::unique_ptr<VertexBuffer> vb);
            void setIndexBuffer(std::unique_ptr<IndexBuffer> ib);

            // Configurar atributos de vértice
            void addAttribute(GLuint index, GLint size, GLenum type, 
                            GLboolean normalized = GL_FALSE, 
                            GLsizei stride = 0, const void* pointer = nullptr);

            // Métodos de conveniencia para tipos comunes
            void addFloatAttribute(GLuint index, GLint size, GLsizei stride = 0, const void* pointer = nullptr);
            void addIntAttribute(GLuint index, GLint size, GLsizei stride = 0, const void* pointer = nullptr);

            GLuint getId() const { return vao_id_; }
            size_t getVertexBufferCount() const { return vertex_buffers_.size(); }
            IndexBuffer* getIndexBuffer() const { return index_buffer_.get(); }
            bool hasIndexBuffer() const { return index_buffer_ != nullptr; }
        };

        // Template implementations
        template<typename T>
        void Buffer::setData(const std::vector<T>& data) {
            setData(data.data(), data.size());
        }

        template<typename T>
        void Buffer::setData(const T* data, size_t count) {
            bind();
            size_ = count * sizeof(T);
            glBufferData(static_cast<GLenum>(type_), size_, data, static_cast<GLenum>(usage_));
        }

        template<typename T>
        void Buffer::updateData(const std::vector<T>& data, size_t offset) {
            bind();
            glBufferSubData(static_cast<GLenum>(type_), offset, data.size() * sizeof(T), data.data());
        }

        template<typename T>
        void IndexBuffer::setIndices(const std::vector<T>& indices) {
            count_ = indices.size();
            setData(indices);
        }

    } // namespace Rendering
} // namespace Graphics

#endif // BUFFER_OBJECTS_H