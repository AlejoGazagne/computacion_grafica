// NEW FILE: src/scene/model.cpp
#include "model.h"
#include "../graphics/shaders/shader_manager.h"
#include <iostream>

namespace Scene
{

  Model::Model(const std::string &name)
      : name_(name), visible_(true)
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

    for (const auto &mesh : meshes_)
    {
      // Configurar si usar textura o no basado en cada mesh
      shader->setBool("useTexture", mesh->hasTexture());
      
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
