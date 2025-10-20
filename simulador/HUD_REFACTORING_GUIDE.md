# Refactorización del Sistema HUD - Guía de Uso

## Resumen de Cambios

Esta refactorización introduce una arquitectura basada en herencia para los instrumentos del HUD, promoviendo la reutilización de código y facilitando la adición de nuevos instrumentos.

## Arquitectura Nueva

### 1. Clase Base: `HUDInstrument`

**Ubicación:** `simulador/src/ui/hud_instrument.h/.cpp`

Clase abstracta que proporciona:

- Gestión automática de VAO/VBO
- Carga de shaders mediante `ShaderManager`
- Administración de dimensiones de pantalla
- Interfaz común para todos los instrumentos

**Métodos principales:**

- `initializeOpenGL(vertex_shader, fragment_shader)`: Inicializa recursos OpenGL
- `cleanup()`: Libera recursos automáticamente
- `updateScreenSize(width, height)`: Actualiza dimensiones
- `render()`: Método abstracto que cada instrumento debe implementar
- `isInitialized()`: Verifica estado de inicialización

### 2. Clase Contenedora: `HUD`

**Ubicación:** `simulador/src/ui/hud.h/.cpp`

Gestiona múltiples instrumentos del HUD:

- Almacena instrumentos como `std::vector<std::unique_ptr<HUDInstrument>>`
- Propaga actualizaciones de tamaño a todos los instrumentos
- Coordina el renderizado de todos los instrumentos

**Métodos principales:**

- `addInstrument(std::unique_ptr<HUDInstrument>)`: Agrega un instrumento
- `updateScreenSize(width, height)`: Propaga a todos los instrumentos
- `render()`: Renderiza todos los instrumentos
- `allInstrumentsReady()`: Verifica que todos estén inicializados
- `getInstrument(index)`: Acceso directo a instrumentos específicos

### 3. Instrumentos Refactorizados

#### `BankAngleIndicator`

- **Hereda de:** `HUDInstrument`
- **Nuevo método:** `setBankAngle(float)` - Configura el ángulo antes de renderizar
- **Cambio:** `render()` ya no recibe parámetros, usa el valor almacenado

#### `PitchLadder`

- **Hereda de:** `HUDInstrument`
- **Nuevo método:** `setPitch(float)` - Configura el pitch antes de renderizar
- **Cambio:** `render()` ya no recibe parámetros, usa el valor almacenado

## Cómo Usar el Nuevo Sistema

### Opción 1: Uso Directo (Compatible con Código Existente)

```cpp
#include "ui/bank_angle.h"
#include "ui/pitch_ladder.h"

// Crear instrumentos individualmente
auto bank_indicator = std::make_unique<UI::BankAngleIndicator>(width, height);
auto pitch_ladder = std::make_unique<UI::PitchLadder>(width, height);

// En el loop de renderizado:
float roll_angle = camera->getRoll();
float pitch_angle = camera->getPitch();

bank_indicator->setBankAngle(roll_angle);
bank_indicator->render();

pitch_ladder->setPitch(pitch_angle);
pitch_ladder->render();

// Actualizar tamaño (e.g., al redimensionar ventana):
bank_indicator->updateScreenSize(new_width, new_height);
pitch_ladder->updateScreenSize(new_width, new_height);
```

### Opción 2: Uso con HUD (Recomendado para Nuevos Proyectos)

```cpp
#include "ui/hud.h"
#include "ui/bank_angle.h"
#include "ui/pitch_ladder.h"

// Crear el sistema HUD
UI::HUD hud;

// Agregar instrumentos al HUD
hud.addInstrument(std::make_unique<UI::BankAngleIndicator>(width, height));
hud.addInstrument(std::make_unique<UI::PitchLadder>(width, height));

// Verificar que todos estén listos
if (!hud.allInstrumentsReady()) {
    std::cerr << "Failed to initialize HUD instruments" << std::endl;
}

// Actualizar datos de instrumentos específicos antes de renderizar
auto* bank_indicator = dynamic_cast<UI::BankAngleIndicator*>(hud.getInstrument(0));
auto* pitch_ladder = dynamic_cast<UI::PitchLadder*>(hud.getInstrument(1));

if (bank_indicator) {
    bank_indicator->setBankAngle(camera->getRoll());
}
if (pitch_ladder) {
    pitch_ladder->setPitch(camera->getPitch());
}

// Renderizar todo el HUD de una vez
hud.render();

// Actualizar tamaño de todos los instrumentos simultáneamente
hud.updateScreenSize(new_width, new_height);
```

### Opción 3: Uso Mejorado con Referencias Mantenidas

```cpp
class FlightSimulator {
private:
    UI::HUD hud_;
    UI::BankAngleIndicator* bank_indicator_;  // Puntero no propietario
    UI::PitchLadder* pitch_ladder_;           // Puntero no propietario

public:
    void initialize(int width, int height) {
        // Crear y agregar instrumentos
        auto bank = std::make_unique<UI::BankAngleIndicator>(width, height);
        auto pitch = std::make_unique<UI::PitchLadder>(width, height);

        // Guardar referencias antes de transferir ownership
        bank_indicator_ = bank.get();
        pitch_ladder_ = pitch.get();

        hud_.addInstrument(std::move(bank));
        hud_.addInstrument(std::move(pitch));
    }

    void render(const Camera& camera) {
        // Actualizar datos
        bank_indicator_->setBankAngle(camera.getRoll());
        pitch_ladder_->setPitch(camera.getPitch());

        // Renderizar todo el HUD
        hud_.render();
    }

    void onResize(int width, int height) {
        hud_.updateScreenSize(width, height);
    }
};
```

## Crear Nuevos Instrumentos

Para agregar un nuevo instrumento al HUD:

```cpp
// 1. Crear header (e.g., altimeter.h)
#include "hud_instrument.h"

namespace UI {
    class Altimeter : public HUDInstrument {
    private:
        float altitude_;

    public:
        Altimeter(int width, int height, const std::string& shader_name = "altimeter_shader");
        ~Altimeter() override = default;

        void setAltitude(float altitude) { altitude_ = altitude; }
        void render() override;
    };
}

// 2. Implementar en .cpp
#include "altimeter.h"

namespace UI {
    Altimeter::Altimeter(int width, int height, const std::string& shader_name)
        : HUDInstrument(width, height, shader_name), altitude_(0.0f) {

        // Inicializar con shaders específicos
        if (!initializeOpenGL("shaders/vertex_altimeter.glsl",
                             "shaders/fragment_altimeter.glsl")) {
            std::cerr << "Failed to initialize Altimeter" << std::endl;
        }
    }

    void Altimeter::render() {
        auto& shader_manager = Graphics::Shaders::ShaderManager::getInstance();
        auto* shader = shader_manager.getShader(shader_name_);
        if (!shader || !shader->isCompiled()) return;

        // Guardar estado GL
        GLboolean depthWasEnabled = glIsEnabled(GL_DEPTH_TEST);
        glDisable(GL_DEPTH_TEST);

        shader->use();
        glBindVertexArray(VAO_);

        // Tu lógica de renderizado aquí usando altitude_
        // ...

        glBindVertexArray(0);
        if (depthWasEnabled) glEnable(GL_DEPTH_TEST);
    }
}

// 3. Agregar al HUD
hud.addInstrument(std::make_unique<UI::Altimeter>(width, height));
```

## Ventajas de la Nueva Arquitectura

1. **Reutilización de Código:** La gestión de VAO/VBO/shaders está centralizada en `HUDInstrument`
2. **Menos Código Duplicado:** No necesitas copiar la inicialización OpenGL en cada instrumento
3. **Fácil Extensión:** Agregar nuevos instrumentos requiere solo implementar `render()`
4. **Gestión Unificada:** La clase `HUD` puede manejar todos los instrumentos de forma consistente
5. **Mantenimiento Simplificado:** Cambios en la gestión de recursos se hacen en un solo lugar
6. **Type Safety:** El uso de polimorfismo garantiza que todos los instrumentos cumplan la interfaz

## Cambios en el Makefile

Se agregaron los nuevos archivos al sistema de compilación:

```makefile
HUD_INSTRUMENT_CXX = ui/hud_instrument
HUD_CXX = ui/hud
# ... y se agregaron a ADDITIONAL_OBJS
```

## Compatibilidad Hacia Atrás

El código existente sigue funcionando sin modificaciones, solo necesita:

1. Cambiar `render(parameter)` por `setParameter(value); render()`
2. Los instrumentos mantienen la misma funcionalidad visible

## Testing

Para verificar que todo funciona correctamente:

```bash
cd simulador
make clean
make -j4
./build/OGL-Engine
```

Los instrumentos deben renderizarse exactamente como antes de la refactorización.
