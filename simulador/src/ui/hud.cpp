#include "hud.h"

namespace UI
{

  size_t HUD::addInstrument(std::unique_ptr<HUDInstrument> instrument)
  {
    instruments_.push_back(std::move(instrument));
    return instruments_.size() - 1;
  }

  void HUD::updateScreenSize(int width, int height)
  {
    for (auto &instrument : instruments_)
    {
      if (instrument)
      {
        instrument->updateScreenSize(width, height);
      }
    }
  }

  void HUD::render()
  {
    // Renderizar cada instrumento en orden
    for (auto &instrument : instruments_)
    {
      if (instrument && instrument->isInitialized())
      {
        instrument->render();
      }
    }
  }

  bool HUD::allInstrumentsReady() const
  {
    for (const auto &instrument : instruments_)
    {
      if (!instrument || !instrument->isInitialized())
      {
        return false;
      }
    }
    return !instruments_.empty(); // También verificar que haya al menos un instrumento
  }

  HUDInstrument *HUD::getInstrument(size_t index)
  {
    if (index < instruments_.size())
    {
      return instruments_[index].get();
    }
    return nullptr;
  }

  void HUD::clear()
  {
    instruments_.clear();
  }

} // namespace UI
