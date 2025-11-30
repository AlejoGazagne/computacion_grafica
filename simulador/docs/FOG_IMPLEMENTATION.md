# Documentación de Implementación de Niebla (Fog)

Este documento detalla la implementación técnica del sistema de niebla volumétrica utilizado en el simulador para mejorar la percepción de profundidad y ocultar los límites de renderizado del terreno.

## 1. Concepto General

El simulador utiliza **Niebla Exponencial Cuadrática Modificada**. A diferencia de la niebla lineal simple, esta fórmula produce una transición más suave y realista, donde la visibilidad decae rápidamente con la distancia pero mantiene cierta claridad en rangos medios.

La fórmula base es:
$$ f = e^{-(d \cdot \rho)^p} $$

Donde:
*   $f$: Factor de niebla (0.0 = todo niebla, 1.0 = sin niebla).
*   $d$: Distancia desde la cámara al fragmento.
*   $\rho$ (rho): Densidad de la niebla.
*   $p$: Exponente de caída (controla la "agresividad" de la curva).

El color final se calcula mezclando el color del objeto con el color de la niebla:
$$ Color_{final} = mix(Color_{niebla}, Color_{objeto}, f) $$

---

## 2. Niebla de Terreno y Objetos 3D

Para los objetos físicos (terreno, avión), la niebla se calcula en función de la **distancia euclidiana** desde la cámara.

### Ubicación del Código
*   **Shader:** `shaders/fragment_3d.glsl`
*   **Configuración:** `src/core/graphics_engine.h` (método `render`)

### Implementación GLSL
```glsl
if (fogEnabled) {
    float distance = length(viewPos - FragPos);
    // Fórmula exponencial modificada con potencia 1.6 para una caída visualmente agradable
    float fogFactor = exp(-pow(distance * fogDensity, 1.6));
    fogFactor = clamp(fogFactor, 0.0, 1.0);
    result = mix(fogColor, result, fogFactor);
}
```

### Parámetros Utilizados
*   `fogDensity`: **0.00006** (para terreno) / **0.0001** (para objetos generales).
    *   *Nota:* El terreno usa una densidad menor para permitir ver más lejos sin perder la sensación atmosférica.
*   `fogColor`: `vec3(0.85, 0.90, 0.95)` (Azul grisáceo claro).

---

## 3. Niebla del Skybox (Horizonte)

La niebla en el skybox es un caso especial. Como el skybox está técnicamente a una distancia "infinita", no podemos usar la distancia de la cámara. En su lugar, simulamos niebla atmosférica basándonos en el **ángulo respecto al horizonte**.

El objetivo es que el cielo se funda suavemente con el color de la niebla en la línea del horizonte, ocultando el borde donde termina el terreno.

### Ubicación del Código
*   **Shader:** `shaders/fragment_skybox.glsl`
*   **Configuración:** `src/graphics/skybox/skybox.cpp` (método `render`)

### Implementación GLSL
```glsl
if (fogEnabled) {
    // 1. Calcular factor de horizonte (0.0 en el cenit/nadir, 1.0 en el horizonte)
    // TexCoords es un vector dirección normalizado. dir.y es la componente vertical.
    vec3 dir = normalize(TexCoords);
    float horizon = 1.0 - abs(dir.y);

    // 2. Aplicar curva exponencial para concentrar la niebla solo cerca del horizonte
    float curved = pow(horizon, skyFogExponent);

    // 3. Calcular cantidad de niebla final
    // Se escala por la densidad global para mantener coherencia con el terreno
    float fogAmount = clamp(curved * fogDensity * skyFogScale, 0.0, skyFogMax);

    // 4. Mezclar color del skybox con color de niebla
    FragColor = vec4(mix(skyboxColor.rgb, fogColor, fogAmount), skyboxColor.a);
}
```

### Parámetros Específicos
*   `skyFogScale`: **15000.0** - Factor de escala masivo para compensar que `fogDensity` es muy pequeño (diseñado para metros) mientras que `curved` es un factor normalizado (0-1).
*   `skyFogExponent`: **1.4** - Controla qué tan rápido desaparece la niebla al mirar hacia arriba. Un valor mayor concentra la niebla más cerca del horizonte.
*   `skyFogMax`: **0.95** - Limita la opacidad máxima de la niebla para que el horizonte nunca sea 100% color plano, manteniendo algo de textura del cielo.

---

## 4. Coherencia Visual

El sistema está diseñado para que `fogColor` sea idéntico en ambos shaders (`vec3(0.85, 0.90, 0.95)`). Esto es crucial:
1.  El terreno lejano se desvanece hacia este color debido a la distancia.
2.  El cielo en el horizonte se desvanece hacia este mismo color debido al ángulo.
3.  **Resultado:** El límite entre el terreno y el cielo se vuelve indistinguible, creando un horizonte infinito y realista.
