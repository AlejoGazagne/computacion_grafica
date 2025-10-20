#ifndef HUD_H
#define HUD_H

#include <memory>
#include <vector>
#include <functional>
#include "hud_instrument.h"

namespace UI
{

  /**
   * @brief Sistema de gestión del HUD (Head-Up Display)
   *
   * Administra múltiples instrumentos del HUD, propagando actualizaciones
   * de tamaño de pantalla y coordinando el renderizado de todos los instrumentos.
   *
   * Uso:
   * ```cpp
   * UI::HUD hud;
   * hud.addInstrument(std::make_unique<UI::BankAngleIndicator>(width, height));
   * hud.addInstrument(std::make_unique<UI::PitchLadder>(width, height));
   *
   * // En el loop de renderizado:
   * hud.updateBankAngle(roll_angle);
   * hud.updatePitch(pitch_angle);
   * hud.render();
   * ```
   */
  class HUD
  {
  private:
    std::vector<std::unique_ptr<HUDInstrument>> instruments_;

    // Callbacks que permiten actualizar datos de instrumentos específicos
    // Esto permite mantener la flexibilidad de pasar parámetros específicos a cada instrumento
    std::vector<std::function<void()>> render_callbacks_;

  public:
    HUD() = default;
    ~HUD() = default;

    // Deshabilitar copia (gestiona punteros únicos)
    HUD(const HUD &) = delete;
    HUD &operator=(const HUD &) = delete;

    /**
     * @brief Agrega un instrumento al HUD
     * @param instrument Puntero único al instrumento a agregar
     * @return Índice del instrumento agregado (útil para referenciarlo después)
     */
    size_t addInstrument(std::unique_ptr<HUDInstrument> instrument);

    /**
     * @brief Propaga actualización de tamaño de pantalla a todos los instrumentos
     * @param width Nuevo ancho de pantalla
     * @param height Nuevo alto de pantalla
     */
    void updateScreenSize(int width, int height);

    /**
     * @brief Renderiza todos los instrumentos del HUD
     *
     * Invoca el método render() de cada instrumento en el orden en que fueron agregados.
     * Cada instrumento debe haber recibido sus datos necesarios previamente mediante
     * los setters correspondientes o callbacks.
     */
    void render();

    /**
     * @brief Obtiene la cantidad de instrumentos registrados
     * @return Número de instrumentos en el HUD
     */
    size_t getInstrumentCount() const { return instruments_.size(); }

    /**
     * @brief Verifica si todos los instrumentos están inicializados correctamente
     * @return true si todos los instrumentos están listos
     */
    bool allInstrumentsReady() const;

    /**
     * @brief Obtiene un instrumento por índice (para configuración avanzada)
     * @param index Índice del instrumento
     * @return Puntero al instrumento o nullptr si el índice es inválido
     */
    HUDInstrument *getInstrument(size_t index);

    /**
     * @brief Limpia todos los instrumentos del HUD
     */
    void clear();
  };

} // namespace UI

#endif // HUD_H
