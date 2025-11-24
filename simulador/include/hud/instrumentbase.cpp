#include "instrumentbase.h"

extern "C"
{
#include <glad/glad.h>
}

#include <iostream>

#include <glm/gtc/matrix_transform.hpp>

namespace hud
{

    InstrumentBase::InstrumentBase(const glm::vec2 &pos, const glm::vec2 &sz, Graphics::Shaders::Shader *shader) : vao_(0), vbo_(0), ebo_(0), shader_(shader)
    {
        position_ = pos;
        size_ = sz;

        model_matrix_ = glm::identity<glm::mat4>();
        projection_matrix_ = glm::identity<glm::mat4>();
    }

    InstrumentBase::~InstrumentBase()
    {
    }

    void InstrumentBase::clean()
    {
        clean_instrument();
        clean_buffers();
    }

    void InstrumentBase::clean_instrument()
    {
    }

    void InstrumentBase::clean_buffers()
    {
        if (vbo_ > 0)
        {
            glDeleteBuffers(1, &vbo_);
            vbo_ = 0;
        }

        if (ebo_ > 0)
        {
            glDeleteBuffers(1, &ebo_);
            ebo_ = 0;
        }

        if (vao_ > 0)
        {
            glDeleteVertexArrays(1, &vao_);
            vao_ = 0;
        }
    }

} // namespace hud
