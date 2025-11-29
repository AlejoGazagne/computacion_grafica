#version 330 core
out vec4 FragColor;

in vec3 ourColor;
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
in vec4 FragPosLightSpace;  // Posición en espacio de luz

uniform sampler2D ourTexture;
uniform sampler2D shadowMap;     // Mapa de profundidad de la luz
uniform bool useTexture;
uniform vec3 viewPos;  // Posición de la cámara para calcular distancia

// Color uniforme (para modelos sin textura)
uniform bool useUniformColor = false;
uniform vec3 uniformColor = vec3(0.5, 0.5, 0.5);

// Niebla
uniform bool fogEnabled = true;
uniform float fogDensity = 0.05;
uniform vec3 fogColor = vec3(0.7, 0.8, 0.9);  // Color azul grisáceo

// Luz direccional (sol)
struct DirLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    bool enabled;
};
uniform DirLight dirLight;

/**
 * Calcula si un fragmento está en sombra usando Shadow Mapping con PCF
 * 
 * Basado en técnica de Shadow Mapping:
 * 1. Transformar coordenadas de clip space [-1,1] a texture space [0,1]
 * 2. Comparar profundidad del fragmento con la profundidad del shadow map
 * 3. Aplicar PCF (Percentage Closer Filtering) para suavizar bordes
 * 
 * @param fragPosLightSpace Posición del fragmento en espacio de luz
 * @param normal Normal del fragmento
 * @param lightDir Dirección de la luz
 * @return Factor de sombra (0.0 = sombra completa, 1.0 = sin sombra)
 */
float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir) {
    // Transformación de perspectiva: dividir por w para obtener NDC [-1, 1]
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    
    // Transformar de NDC [-1,1] a espacio de textura [0,1]
    projCoords = projCoords * 0.5 + 0.5;
    
    // Fuera del frustum de luz = sin sombra
    if (projCoords.z > 1.0)
        return 0.0;
    
    // Profundidad actual del fragmento en espacio de luz
    float currentDepth = projCoords.z;
    
    // Bias para evitar shadow acne (patrones de bandas)
    // Mayor bias con ángulos más oblicuos
    float bias = max(0.005 * (1.0 - dot(normal, lightDir)), 0.001);
    
    // PCF (Percentage Closer Filtering) para suavizar bordes
    // Muestrea 9 texels vecinos (kernel 3x3) y promedia
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);  // Tamaño de un texel
    
    for(int x = -1; x <= 1; ++x) {
        for(int y = -1; y <= 1; ++y) {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;  // Promedio de 9 muestras
    
    return shadow;
}

// Función para calcular luz direccional con sombras
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 baseColor) {
    if (!light.enabled) return vec3(0.0);
    
    vec3 lightDir = normalize(-light.direction);
    
    // Ambient - luz base para que todo sea visible (sin sombras)
    vec3 ambient = light.ambient * baseColor;
    
    // Diffuse - iluminación simple basada en el ángulo
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * baseColor;
    
    // Calcular sombras
    float shadow = ShadowCalculation(FragPosLightSpace, normal, lightDir);
    
    // Aplicar sombras solo a diffuse (ambient no se ve afectado)
    return ambient + (1.0 - shadow) * diffuse;
}

void main() {
    vec3 norm = normalize(Normal);
    
    // Obtener color base
    vec3 baseColor;
    float alpha = 1.0;
    
    if (useUniformColor) {
        // Usar color uniforme
        baseColor = uniformColor;
    } else if (useTexture) {
        vec4 texColor = texture(ourTexture, TexCoords);
        baseColor = texColor.rgb;
        alpha = texColor.a;
    } else {
        baseColor = ourColor;
    }
    
    // Calcular iluminación
    vec3 result = vec3(0.0);
    
    // Luz direccional (sol)
    result += CalcDirLight(dirLight, norm, baseColor);
    
    // Clamp para evitar saturación
    result = clamp(result, 0.0, 1.0);
    
    // Aplicar niebla
    if (fogEnabled) {
        float distance = length(viewPos - FragPos);
        float fogFactor = exp(-pow(distance * fogDensity, 1.6));
        fogFactor = clamp(fogFactor, 0.0, 1.0);
        result = mix(fogColor, result, fogFactor);
    }
    
    FragColor = vec4(result, alpha);
}
