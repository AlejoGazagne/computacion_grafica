/**
 * @file hud_usage_example.cpp
 * @brief Ejemplo completo de cómo usar el nuevo sistema HUD refactorizado
 *
 * Este archivo muestra tres formas de usar los instrumentos del HUD:
 * 1. Uso directo individual (compatible con código legacy)
 * 2. Uso con la clase HUD (recomendado)
 * 3. Patrón completo con gestión centralizada
 */

#include "ui/hud.h"
#include "ui/bank_angle.h"
#include "ui/pitch_ladder.h"
#include "scene/camera.h"
#include <memory>
#include <iostream>

// ============================================================================
// EJEMPLO 1: Uso Directo Individual (Backward Compatible)
// ============================================================================

class FlightSimulator_Legacy
{
private:
  std::unique_ptr<UI::BankAngleIndicator> bank_indicator_;
  std::unique_ptr<UI::PitchLadder> pitch_ladder_;

public:
  void initialize(int width, int height)
  {
    // Crear instrumentos individualmente
    bank_indicator_ = std::make_unique<UI::BankAngleIndicator>(width, height);
    pitch_ladder_ = std::make_unique<UI::PitchLadder>(width, height);

    // Verificar inicialización
    if (!bank_indicator_->isInitialized())
    {
      std::cerr << "Failed to initialize BankAngleIndicator" << std::endl;
    }
    if (!pitch_ladder_->isInitialized())
    {
      std::cerr << "Failed to initialize PitchLadder" << std::endl;
    }
  }

  void render(float roll_angle, float pitch_angle)
  {
    // Configurar datos antes de renderizar
    if (bank_indicator_ && bank_indicator_->isInitialized())
    {
      bank_indicator_->setBankAngle(roll_angle);
      bank_indicator_->render();
    }

    if (pitch_ladder_ && pitch_ladder_->isInitialized())
    {
      pitch_ladder_->setPitch(pitch_angle);
      pitch_ladder_->render();
    }
  }

  void onWindowResize(int width, int height)
  {
    // Actualizar cada instrumento individualmente
    if (bank_indicator_)
    {
      bank_indicator_->updateScreenSize(width, height);
    }
    if (pitch_ladder_)
    {
      pitch_ladder_->updateScreenSize(width, height);
    }
  }
};

// ============================================================================
// EJEMPLO 2: Uso con Clase HUD (Recomendado para Nuevos Proyectos)
// ============================================================================

class FlightSimulator_WithHUD
{
private:
  UI::HUD hud_;
  // Mantener referencias no propietarias para acceso rápido
  UI::BankAngleIndicator *bank_indicator_;
  UI::PitchLadder *pitch_ladder_;

public:
  void initialize(int width, int height)
  {
    // Crear instrumentos
    auto bank = std::make_unique<UI::BankAngleIndicator>(width, height);
    auto pitch = std::make_unique<UI::PitchLadder>(width, height);

    // Guardar referencias antes de transferir ownership
    bank_indicator_ = bank.get();
    pitch_ladder_ = pitch.get();

    // Agregar al HUD (el HUD toma ownership)
    hud_.addInstrument(std::move(bank));
    hud_.addInstrument(std::move(pitch));

    // Verificación centralizada
    if (!hud_.allInstrumentsReady())
    {
      std::cerr << "Some HUD instruments failed to initialize" << std::endl;
    }
    else
    {
      std::cout << "HUD initialized successfully with "
                << hud_.getInstrumentCount() << " instruments" << std::endl;
    }
  }

  void render(float roll_angle, float pitch_angle)
  {
    // Actualizar datos de instrumentos
    if (bank_indicator_)
    {
      bank_indicator_->setBankAngle(roll_angle);
    }
    if (pitch_ladder_)
    {
      pitch_ladder_->setPitch(pitch_angle);
    }

    // Renderizar todo el HUD de una vez
    hud_.render();
  }

  void onWindowResize(int width, int height)
  {
    // Una sola llamada actualiza todos los instrumentos
    hud_.updateScreenSize(width, height);
  }

  // Método adicional: acceso seguro a instrumentos por índice
  void configureInstrument(size_t index)
  {
    if (auto *instrument = hud_.getInstrument(index))
    {
      // Configuración específica del instrumento
      std::cout << "Configuring instrument: " << instrument->getShaderName() << std::endl;
    }
  }
};

// ============================================================================
// EJEMPLO 3: Patrón Completo con Gestión Centralizada
// ============================================================================

/**
 * @brief Sistema de HUD completamente integrado en un motor gráfico
 */
class AdvancedFlightSimulator
{
private:
  UI::HUD hud_;

  // Estructura para mantener referencias tipadas a instrumentos específicos
  struct InstrumentRefs
  {
    UI::BankAngleIndicator *bank_angle = nullptr;
    UI::PitchLadder *pitch_ladder = nullptr;
    // Agregar más instrumentos según sea necesario
  } instruments_;

  bool first_person_mode_ = false;

public:
  /**
   * @brief Inicializa el sistema HUD completo
   */
  bool initializeHUD(int width, int height)
  {
    // Crear y agregar Bank Angle Indicator
    auto bank = std::make_unique<UI::BankAngleIndicator>(width, height);
    instruments_.bank_angle = bank.get();
    hud_.addInstrument(std::move(bank));

    // Crear y agregar Pitch Ladder
    auto pitch = std::make_unique<UI::PitchLadder>(width, height);
    instruments_.pitch_ladder = pitch.get();
    hud_.addInstrument(std::move(pitch));

    // Aquí podrías agregar más instrumentos:
    // auto altimeter = std::make_unique<UI::Altimeter>(width, height);
    // instruments_.altimeter = altimeter.get();
    // hud_.addInstrument(std::move(altimeter));

    // Verificar que todo se inicializó correctamente
    if (!hud_.allInstrumentsReady())
    {
      std::cerr << "ERROR: Failed to initialize HUD instruments" << std::endl;
      return false;
    }

    std::cout << "HUD System initialized: " << hud_.getInstrumentCount()
              << " instruments ready" << std::endl;
    return true;
  }

  /**
   * @brief Actualiza y renderiza el HUD basado en el estado de la cámara
   */
  void updateAndRenderHUD(const Scene::Camera &camera)
  {
    // Solo renderizar HUD en modo primera persona
    if (!first_person_mode_)
    {
      return;
    }

    // Actualizar datos de instrumentos desde la cámara
    updateInstrumentData(camera);

    // Renderizar todo el HUD
    hud_.render();
  }

  /**
   * @brief Actualiza los datos de todos los instrumentos
   */
  void updateInstrumentData(const Scene::Camera &camera)
  {
    // Bank Angle Indicator
    if (instruments_.bank_angle)
    {
      float roll = camera.getRoll();
      instruments_.bank_angle->setBankAngle(roll);
    }

    // Pitch Ladder
    if (instruments_.pitch_ladder)
    {
      float pitch = camera.getPitch();
      instruments_.pitch_ladder->setPitch(pitch);
    }

    // Actualizar otros instrumentos según sea necesario
    // if (instruments_.altimeter) {
    //     instruments_.altimeter->setAltitude(camera.getPosition().y);
    // }
  }

  /**
   * @brief Maneja redimensionamiento de ventana
   */
  void handleWindowResize(int width, int height)
  {
    hud_.updateScreenSize(width, height);
    std::cout << "HUD resized to: " << width << "x" << height << std::endl;
  }

  /**
   * @brief Alterna entre modo primera persona y tercera persona
   */
  void toggleCameraMode()
  {
    first_person_mode_ = !first_person_mode_;
    std::cout << "Camera mode: " << (first_person_mode_ ? "First Person" : "Third Person")
              << " (HUD " << (first_person_mode_ ? "enabled" : "disabled") << ")" << std::endl;
  }

  /**
   * @brief Limpia el sistema HUD
   */
  void cleanup()
  {
    hud_.clear();
    instruments_ = InstrumentRefs{}; // Reset referencias
    std::cout << "HUD System cleaned up" << std::endl;
  }

  // Getters
  bool isFirstPersonMode() const { return first_person_mode_; }
  const UI::HUD &getHUD() const { return hud_; }
};

// ============================================================================
// EJEMPLO 4: Uso Dinámico - Agregar/Quitar Instrumentos en Runtime
// ============================================================================

class DynamicHUDExample
{
private:
  UI::HUD hud_;
  std::vector<std::string> instrument_names_;

public:
  void initialize(int width, int height)
  {
    std::cout << "Starting with empty HUD..." << std::endl;
  }

  void addBankAngleIndicator(int width, int height)
  {
    auto instrument = std::make_unique<UI::BankAngleIndicator>(width, height);
    if (instrument->isInitialized())
    {
      instrument_names_.push_back("Bank Angle Indicator");
      hud_.addInstrument(std::move(instrument));
      std::cout << "Added Bank Angle Indicator. Total instruments: "
                << hud_.getInstrumentCount() << std::endl;
    }
  }

  void addPitchLadder(int width, int height)
  {
    auto instrument = std::make_unique<UI::PitchLadder>(width, height);
    if (instrument->isInitialized())
    {
      instrument_names_.push_back("Pitch Ladder");
      hud_.addInstrument(std::move(instrument));
      std::cout << "Added Pitch Ladder. Total instruments: "
                << hud_.getInstrumentCount() << std::endl;
    }
  }

  void clearAllInstruments()
  {
    hud_.clear();
    instrument_names_.clear();
    std::cout << "All instruments removed" << std::endl;
  }

  void listInstruments() const
  {
    std::cout << "Active HUD Instruments (" << hud_.getInstrumentCount() << "):" << std::endl;
    for (size_t i = 0; i < instrument_names_.size(); ++i)
    {
      std::cout << "  [" << i << "] " << instrument_names_[i] << std::endl;
    }
  }
};

// ============================================================================
// MAIN: Demostración de los diferentes enfoques
// ============================================================================

void demonstrateUsagePatterns()
{
  const int SCREEN_WIDTH = 1920;
  const int SCREEN_HEIGHT = 1080;

  std::cout << "========================================" << std::endl;
  std::cout << "HUD System Usage Examples" << std::endl;
  std::cout << "========================================\n"
            << std::endl;

  // Ejemplo 1: Legacy/Directo
  std::cout << "--- Example 1: Direct Usage (Legacy Compatible) ---" << std::endl;
  FlightSimulator_Legacy legacy_sim;
  legacy_sim.initialize(SCREEN_WIDTH, SCREEN_HEIGHT);
  legacy_sim.render(15.5f, -5.2f); // roll=15.5°, pitch=-5.2°
  legacy_sim.onWindowResize(1280, 720);
  std::cout << std::endl;

  // Ejemplo 2: Con HUD
  std::cout << "--- Example 2: Using HUD Class ---" << std::endl;
  FlightSimulator_WithHUD hud_sim;
  hud_sim.initialize(SCREEN_WIDTH, SCREEN_HEIGHT);
  hud_sim.render(15.5f, -5.2f);
  hud_sim.onWindowResize(1280, 720);
  hud_sim.configureInstrument(0); // Configurar primer instrumento
  std::cout << std::endl;

  // Ejemplo 3: Sistema Avanzado
  std::cout << "--- Example 3: Advanced Integrated System ---" << std::endl;
  AdvancedFlightSimulator advanced_sim;
  advanced_sim.initializeHUD(SCREEN_WIDTH, SCREEN_HEIGHT);
  advanced_sim.toggleCameraMode(); // Activar primera persona

  // Simular actualización con una cámara mock
  // En un caso real, pasarías tu objeto Scene::Camera
  // advanced_sim.updateAndRenderHUD(camera);

  advanced_sim.handleWindowResize(1280, 720);
  advanced_sim.cleanup();
  std::cout << std::endl;

  // Ejemplo 4: Gestión Dinámica
  std::cout << "--- Example 4: Dynamic Instrument Management ---" << std::endl;
  DynamicHUDExample dynamic_hud;
  dynamic_hud.initialize(SCREEN_WIDTH, SCREEN_HEIGHT);
  dynamic_hud.addBankAngleIndicator(SCREEN_WIDTH, SCREEN_HEIGHT);
  dynamic_hud.addPitchLadder(SCREEN_WIDTH, SCREEN_HEIGHT);
  dynamic_hud.listInstruments();
  dynamic_hud.clearAllInstruments();
  std::cout << std::endl;

  std::cout << "========================================" << std::endl;
  std::cout << "All examples completed successfully!" << std::endl;
  std::cout << "========================================" << std::endl;
}

// Nota: Este es un archivo de ejemplo/documentación.
// No debe ser compilado como parte del proyecto principal.
// Su propósito es mostrar patrones de uso del sistema HUD refactorizado.
