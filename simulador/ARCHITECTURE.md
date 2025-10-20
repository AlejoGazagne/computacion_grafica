# Arquitectura del Sistema HUD Refactorizado

## Diagrama de Clases

```
┌─────────────────────────────────────┐
│       HUDInstrument (Abstract)      │
├─────────────────────────────────────┤
│ # VAO_: GLuint                      │
│ # VBO_: GLuint                      │
│ # shader_name_: string              │
│ # screen_width_: int                │
│ # screen_height_: int               │
├─────────────────────────────────────┤
│ # initializeOpenGL(vs, fs): bool    │
│ # cleanup(): void                   │
│ + updateScreenSize(w, h): void      │
│ + render(): void [abstract]         │
│ + isInitialized(): bool             │
│ + getShaderName(): string           │
└─────────────────────────────────────┘
                  △
                  │ inherits
         ┌────────┴────────┐
         │                 │
┌────────┴──────────┐  ┌───┴──────────────┐
│ BankAngleIndicator│  │   PitchLadder    │
├───────────────────┤  ├──────────────────┤
│ - bank_angle_     │  │ - camera_pitch_  │
├───────────────────┤  ├──────────────────┤
│ + setBankAngle()  │  │ + setPitch()     │
│ + render()        │  │ + render()       │
└───────────────────┘  └──────────────────┘
         │                 │
         └────────┬────────┘
                  │ managed by
                  ▼
         ┌────────────────────────────┐
         │           HUD              │
         ├────────────────────────────┤
         │ - instruments_: vector<    │
         │     unique_ptr<            │
         │       HUDInstrument>>      │
         ├────────────────────────────┤
         │ + addInstrument()          │
         │ + render()                 │
         │ + updateScreenSize()       │
         │ + allInstrumentsReady()    │
         │ + getInstrument(i)         │
         │ + clear()                  │
         └────────────────────────────┘
```

## Jerarquía de Archivos

```
simulador/
├── src/
│   └── ui/
│       ├── hud_instrument.h          [NUEVO] Clase base abstracta
│       ├── hud_instrument.cpp        [NUEVO] Implementación base
│       ├── hud.h                     [NUEVO] Gestor de instrumentos
│       ├── hud.cpp                   [NUEVO] Implementación HUD
│       ├── bank_angle.h              [MODIFICADO] Hereda de HUDInstrument
│       ├── bank_angle.cpp            [MODIFICADO] Simplificado
│       ├── pitch_ladder.h            [MODIFICADO] Hereda de HUDInstrument
│       └── pitch_ladder.cpp          [MODIFICADO] Simplificado
├── examples/
│   └── hud_usage_example.cpp         [NUEVO] Ejemplos de uso
├── HUD_REFACTORING_GUIDE.md          [NUEVO] Guía de uso
├── REFACTORING_CHANGES.md            [NUEVO] Resumen de cambios
└── Makefile                          [MODIFICADO] Agrega nuevos archivos
```

## Flujo de Datos

### Inicialización

```
main.cpp:
  └─> BankAngleIndicator::constructor(width, height, shader_name)
        └─> HUDInstrument::constructor(width, height, shader_name)
              │ Almacena: screen_width_, screen_height_, shader_name_
              │ Inicializa: VAO_ = 0, VBO_ = 0
              └─> return to BankAngleIndicator
        └─> BankAngleIndicator::initializeOpenGL(vertex_shader, fragment_shader)
              └─> HUDInstrument::initializeOpenGL(vs, fs)
                    ├─> ShaderManager::loadShader(shader_name_, vs, fs)
                    ├─> glGenVertexArrays(&VAO_)
                    ├─> glGenBuffers(&VBO_)
                    └─> Configure vertex attributes
```

### Renderizado

```
main.cpp (loop):
  ├─> camera->getRoll() → roll_angle
  ├─> bank_indicator_->setBankAngle(roll_angle)
  │     └─> Almacena en bank_angle_
  └─> bank_indicator_->render()
        ├─> Obtiene shader desde ShaderManager
        ├─> Configura estado OpenGL
        ├─> Usa bank_angle_ para generar geometría
        ├─> Actualiza VBO_ con datos
        ├─> glDrawArrays()
        └─> Restaura estado OpenGL
```

### Uso con HUD

```
main.cpp:
  └─> hud.addInstrument(std::make_unique<BankAngleIndicator>(...))
        └─> instruments_.push_back(std::move(instrument))

  └─> hud.updateScreenSize(width, height)
        └─> for each instrument in instruments_:
              └─> instrument->updateScreenSize(width, height)

  └─> hud.render()
        └─> for each instrument in instruments_:
              └─> if (instrument->isInitialized())
                    └─> instrument->render()
```

## Responsabilidades de Cada Clase

### HUDInstrument (Clase Base)

**Responsabilidades:**

- ✅ Crear y destruir VAO/VBO
- ✅ Cargar shaders mediante ShaderManager
- ✅ Almacenar dimensiones de pantalla
- ✅ Proporcionar interfaz común

**No responsable de:**

- ❌ Lógica de renderizado específica (delegada a clases derivadas)
- ❌ Gestión de datos específicos de instrumentos (bank angle, pitch, etc.)
- ❌ Generación de geometría concreta

### BankAngleIndicator / PitchLadder

**Responsabilidades:**

- ✅ Implementar `render()` con lógica específica
- ✅ Almacenar y gestionar sus datos específicos (bank*angle*, camera*pitch*)
- ✅ Generar geometría apropiada para su visualización
- ✅ Llamar a `initializeOpenGL()` con rutas de shaders correctas

**No responsable de:**

- ❌ Gestión de recursos OpenGL (delegado a clase base)
- ❌ Carga de shaders (delegado a clase base)

### HUD

**Responsabilidades:**

- ✅ Almacenar y gestionar lifetime de instrumentos
- ✅ Propagar eventos (resize, etc.) a todos los instrumentos
- ✅ Coordinar renderizado de múltiples instrumentos
- ✅ Proporcionar acceso a instrumentos individuales

**No responsable de:**

- ❌ Conocer detalles de implementación de instrumentos específicos
- ❌ Actualizar datos de instrumentos (responsabilidad del código cliente)

## Patrones de Diseño Utilizados

### 1. Template Method Pattern

```cpp
// HUDInstrument define el esqueleto del algoritmo
class HUDInstrument {
protected:
    bool initializeOpenGL(vs, fs) {
        // Pasos comunes de inicialización
        loadShaders();
        createBuffers();
        configureAttributes();
    }
public:
    virtual void render() = 0; // Paso específico por subclase
};
```

### 2. RAII (Resource Acquisition Is Initialization)

```cpp
HUDInstrument::HUDInstrument(...) {
    // Adquisición de recursos en constructor
}

HUDInstrument::~HUDInstrument() {
    cleanup(); // Liberación automática en destructor
}
```

### 3. Composition Over Inheritance

```cpp
class HUD {
    std::vector<std::unique_ptr<HUDInstrument>> instruments_;
    // HUD "tiene" instrumentos en lugar de "ser" un instrumento
};
```

### 4. Factory Pattern (Implícito)

```cpp
// El cliente puede crear instrumentos polimórficamente
auto instrument = std::make_unique<BankAngleIndicator>(...);
hud.addInstrument(std::move(instrument));
```

## Ventajas de la Arquitectura

### 1. Separación de Concerns

```
┌──────────────────────┐
│  Gestión de Recursos │ ← HUDInstrument
├──────────────────────┤
│  Lógica de Negocio   │ ← BankAngleIndicator, PitchLadder
├──────────────────────┤
│  Coordinación        │ ← HUD
└──────────────────────┘
```

### 2. Open/Closed Principle

- **Abierto para extensión:** Agregar nuevos instrumentos es fácil
- **Cerrado para modificación:** No necesitas modificar HUDInstrument o HUD

### 3. Single Responsibility Principle

Cada clase tiene una única razón para cambiar:

- `HUDInstrument`: Cambios en gestión de recursos OpenGL
- `BankAngleIndicator`: Cambios en visualización de bank angle
- `HUD`: Cambios en coordinación de múltiples instrumentos

### 4. Dependency Inversion Principle

```cpp
// HUD depende de la abstracción (HUDInstrument)
// no de implementaciones concretas (BankAngleIndicator)
class HUD {
    std::vector<std::unique_ptr<HUDInstrument>> instruments_;
    //                         ^^^^^^^^^^^^^ Abstracción
};
```

## Métricas de Código

### Antes de la Refactorización

```
BankAngleIndicator:   ~250 líneas (incluyendo gestión OpenGL)
PitchLadder:         ~200 líneas (incluyendo gestión OpenGL)
Total:               ~450 líneas
Código duplicado:    ~60 líneas (gestión VAO/VBO/shaders)
```

### Después de la Refactorización

```
HUDInstrument:       ~100 líneas (base compartida)
HUD:                 ~80 líneas (gestión de instrumentos)
BankAngleIndicator:  ~190 líneas (solo lógica específica)
PitchLadder:         ~140 líneas (solo lógica específica)
Total:               ~510 líneas
Código duplicado:    0 líneas
```

**Análisis:**

- ✅ +60 líneas totales PERO código más organizado y mantenible
- ✅ 100% eliminación de código duplicado
- ✅ Facilita agregar N instrumentos nuevos con ~100 líneas c/u vs ~250 antes

## Extensibilidad: Agregar Nuevo Instrumento

### Tiempo estimado: 30 minutos (vs 1-2 horas antes)

1. **Crear header** (~5 min)

```cpp
class Altimeter : public HUDInstrument {
    float altitude_;
public:
    Altimeter(int w, int h);
    void setAltitude(float alt) { altitude_ = alt; }
    void render() override;
};
```

2. **Implementar constructor** (~5 min)

```cpp
Altimeter::Altimeter(int w, int h)
    : HUDInstrument(w, h, "altimeter_shader"), altitude_(0.0f) {
    initializeOpenGL("shaders/vertex_alt.glsl", "shaders/fragment_alt.glsl");
}
```

3. **Implementar render()** (~15 min)

```cpp
void Altimeter::render() {
    // Tu lógica aquí, usando altitude_
}
```

4. **Agregar al Makefile** (~2 min)

```makefile
ALTIMETER_CXX = ui/altimeter
ADDITIONAL_OBJS = ... $(BUILD_DIR)/$(ALTIMETER_CXX).o
```

5. **Usar en main.cpp** (~3 min)

```cpp
hud.addInstrument(std::make_unique<UI::Altimeter>(width, height));
```

✅ **Total:** ~30 minutos, 100% enfocado en lógica del instrumento

## Testing y Validación

### Checklist de Validación

- ✅ Compila sin errores
- ✅ No hay warnings críticos
- ✅ Comportamiento visual idéntico a versión anterior
- ✅ No hay fugas de memoria (verificado con Valgrind/sanitizers)
- ✅ Rendimiento equivalente (sin overhead adicional)

### Pruebas Realizadas

```bash
$ make clean && make -j4
# ✅ Compilación exitosa

$ ./build/OGL-Engine
# ✅ Instrumentos renderizan correctamente
# ✅ Cambio de tamaño de ventana funciona
# ✅ Alternancia primera/tercera persona funciona
```

## Conclusión

Esta refactorización establece una **base sólida y escalable** para el sistema HUD:

1. **Código más limpio:** Eliminación de duplicación
2. **Más mantenible:** Cambios centralizados
3. **Más extensible:** Agregar instrumentos es trivial
4. **Más testeable:** Cada clase tiene responsabilidad única
5. **Backward compatible:** Migración gradual posible

El sistema está listo para crecer con nuevos instrumentos de forma eficiente y organizada.
