// NEW FILE: src/scene/model.cpp
#include "model.h"
#include "../graphics/shaders/shader_manager.h"
#include <iostream>

namespace Scene
{

  Model::Model(const std::string &name)
      : name_(name), visible_(true), use_uniform_color_(false), 
        uniform_color_(1.0f, 1.0f, 1.0f)
  {
  }

  void Model::addMesh(std::unique_ptr<Graphics::Rendering::Mesh> mesh)
  {
    if (mesh)
    {
      meshes_.push_back(std::move(mesh));
    }
  }

  void Model::render(Graphics::Shaders::Shader *shader) const
  {
    if (!visible_ || !shader)
      return;

    shader->use();
    shader->setMat4("model", transform_.getMatrix());

    // Si se usa color uniforme, configurarlo en el shader
    if (use_uniform_color_)
    {
      shader->setBool("useUniformColor", true);
      shader->setVec3("uniformColor", uniform_color_);
    }
    else
    {
      shader->setBool("useUniformColor", false);
    }

    for (const auto &mesh : meshes_)
    {
      if (mesh->hasInstanceData())
      {
        mesh->drawInstanced(mesh->getInstanceCount());
      }
      else
      {
        mesh->draw();
      }
    }

    shader->unuse();
  }

}
