// NEW FILE: src/scene/model.h
#ifndef MODEL_H
#define MODEL_H

#include "../graphics/rendering/buffer_objects.h"
#include "mesh.h"
#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include <string>

namespace Scene
{

  struct Transform
  {
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 rotation = glm::vec3(0.0f); // Euler angles en radianes
    glm::vec3 scale = glm::vec3(1.0f);

    glm::mat4 getMatrix() const
    {
      glm::mat4 mat = glm::mat4(1.0f);
      mat = glm::translate(mat, position);
      mat = glm::rotate(mat, rotation.y, glm::vec3(0, 1, 0)); // Yaw
      mat = glm::rotate(mat, rotation.x, glm::vec3(1, 0, 0)); // Pitch
      mat = glm::rotate(mat, rotation.z, glm::vec3(0, 0, 1)); // Roll
      mat = glm::scale(mat, scale);
      return mat;
    }
  };

  class Model
  {
  private:
    std::vector<std::unique_ptr<Graphics::Rendering::Mesh>> meshes_;
    Transform transform_;
    std::string name_;
    bool visible_;

  public:
    Model(const std::string &name = "model");
    ~Model() = default;

    // No copiable
    Model(const Model &) = delete;
    Model &operator=(const Model &) = delete;

    // Movible
    Model(Model &&) = default;
    Model &operator=(Model &&) = default;

    void addMesh(std::unique_ptr<Graphics::Rendering::Mesh> mesh);
    void render(Graphics::Shaders::Shader *shader) const;

    // Getters
    const std::string &getName() const { return name_; }
    const Transform &getTransform() const { return transform_; }
    bool isVisible() const { return visible_; }
    size_t getMeshCount() const { return meshes_.size(); }

    // Setters
    Transform &getTransform() { return transform_; }
    void setVisible(bool visible) { visible_ = visible; }
  };

}

#endif // MODEL_H
