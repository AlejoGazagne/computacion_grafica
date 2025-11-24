#ifndef INSTRUMENTBASE_H
#define INSTRUMENTBASE_H

#include <glm/glm.hpp>

#include "huddef.h"

// Forward declaration of engine Shader in the correct namespace
namespace Graphics
{
    namespace Shaders
    {
        class Shader;
    }
}

namespace hud
{

    class InstrumentBase
    {
    public:
        InstrumentBase(const glm::vec2 &pos, const glm::vec2 &sz, Graphics::Shaders::Shader *shader);

        virtual ~InstrumentBase();

        virtual void initialize() = 0;
        virtual void update(const FlightData &data) = 0;
        virtual void render(void) = 0;

        ///
        /// \brief Set instrument position in normalized viewport
        /// \param pos
        ///
        inline void set_position(const glm::vec2 &pos)
        {
            position_ = pos;
        }

        ///
        /// \brief Set instrument size
        /// \param sz
        ///
        inline void set_size(const glm::vec2 &sz)
        {
            size_ = sz;
        }

        inline void set_projection(const glm::mat4 &p)
        {
            projection_matrix_ = p;
        }

        inline glm::vec2 get_position() const { return position_; }
        inline glm::vec2 get_size() const { return size_; }

        ///
        /// \brief Release instrument resources and buffers. Must be called before
        /// object deletion
        ///
        void clean(void);

    protected:
        // Subclass utilities
        virtual void updateModelMatrix() = 0;

        ///
        /// \brief Could be overrrided by derived classes for specific cleaning operations
        ///
        virtual void clean_instrument(void);

        ///
        /// \brief frees opengl buffers. should be called before deleting the object
        ///
        void clean_buffers();

    protected:
        unsigned vao_, vbo_, ebo_; /// OpenGL objects
        Graphics::Shaders::Shader *shader_;

        glm::vec2 position_; /// Position in viewport position (normalized)
        glm::vec2 size_;     /// Instrument size

        glm::mat4 model_matrix_;
        glm::mat4 projection_matrix_;
    };

} // End namespace hud

#endif // INSTRUMENTBASE_H
