# Cámara Cinemática de Espectador

## Descripción

Se ha implementado una nueva cámara cinemática de espectador que se posiciona por delante y al costado de la trayectoria del avión, creando un efecto visual similar a un espectador parado en la vereda mirando pasar un vehículo por la calle.

## Características

### Comportamiento de la Cámara

1. **Posicionamiento**: La cámara se coloca por delante del avión en su trayectoria de movimiento, pero con un offset lateral (desplazada hacia un costado).

2. **Apuntado**: La cámara siempre mira hacia el avión (usando `lookAt`), manteniéndolo en el encuadre.

3. **Flujo Visual**:

   - La cámara espera posicionada por delante y al costado de la trayectoria del avión
   - El avión se acerca, pasa frente a la cámara y se aleja
   - Cuando el avión se aleja más allá de un umbral configurable, la cámara se reposiciona automáticamente para el siguiente "corte"

4. **Reposicionamiento Automático**: Cuando la distancia entre la cámara y el avión supera el umbral configurado, la cámara se reposiciona instantáneamente (o con suavizado) en una nueva posición por delante del avión.

## Controles

### Activación/Desactivación

- **Tecla C**: Alterna entre primera persona y tercera persona
- **Tecla V**: Alterna entre modo cinemático y tercera persona clásica (solo funciona cuando está activa la tercera persona)

### Ajustes en Tiempo Real

Cuando la cámara cinemática está activa, puedes ajustar los siguientes parámetros:

- **Tecla 3**: Incrementa la distancia por delante del avión (50-300m)
- **Tecla 4**: Incrementa el offset lateral (-100 a 100m)
- **Tecla 5**: Incrementa el offset de altura (0-100m)
- **Tecla 6**: Incrementa la distancia máxima antes de reposicionar (100-500m)

Cada presión de tecla incrementa el valor correspondiente. Los valores están limitados a rangos seguros.

## Parámetros Configurables

Los parámetros por defecto se pueden modificar en `src/main.cpp`:

```cpp
bool cinematic_mode_ = true;                       // Activar/desactivar modo cinemático
float cinematic_forward_distance_ = 100.0f;        // Distancia por delante del avión (metros)
float cinematic_lateral_offset_ = 50.0f;           // Offset lateral (+ = derecha, - = izquierda)
float cinematic_height_offset_ = 20.0f;            // Offset en altura (metros)
float cinematic_max_distance_ = 150.0f;            // Distancia máxima antes de reposicionar
float cinematic_smooth_factor_ = 0.0f;             // Suavizado (0=instantáneo, 1=muy suave)
```

### Descripción de Parámetros

- **cinematic*forward_distance***: Determina qué tan adelante del avión se coloca la cámara en su trayectoria
- **cinematic*lateral_offset***: Desplazamiento lateral desde el eje del avión (positivo = derecha, negativo = izquierda)
- **cinematic*height_offset***: Altura adicional respecto a la posición del avión
- **cinematic*max_distance***: Umbral de distancia que activa el reposicionamiento automático
- **cinematic*smooth_factor***: Controla la interpolación del movimiento de cámara (0 = teleport instantáneo, valores más altos = transición suave)

## Implementación Técnica

### Cálculo de Posición

La posición de la cámara se calcula de la siguiente manera:

1. Se obtienen los vectores del avión (forward, right, up) basados en sus ángulos de Euler
2. Se calcula un punto por delante: `punto_delante = posicion_avion + forward * distancia_delantera`
3. Se aplica offset lateral: `posicion_camara = punto_delante + right * offset_lateral`
4. Se aplica offset de altura: `posicion_camara += up * offset_altura`
5. La cámara apunta al avión usando `lookAt(posicion_avion)`

### Detección de Reposicionamiento

Se calcula continuamente la distancia entre la cámara y el avión. Cuando esta distancia supera `cinematic_max_distance_`, se recalcula la posición de la cámara usando los vectores actuales del avión.

## Diferencias con Tercera Persona Clásica

| Característica | Cámara Cinemática           | Tercera Persona Clásica |
| -------------- | --------------------------- | ----------------------- |
| Posición       | Por delante y al costado    | Detrás del avión        |
| Movimiento     | Reposicionamiento periódico | Sigue continuamente     |
| Perspectiva    | Espectador estático         | Pegada al avión         |
| Uso            | Vistas cinematográficas     | Control del avión       |

## Notas Importantes

⚠️ **La física y controles del avión NO se modifican**. Solo cambia la posición y orientación de la cámara.

- El avión sigue respondiendo a los controles normalmente
- La cámara es puramente observacional
- Ideal para grabar videos o tomas cinematográficas del vuelo
