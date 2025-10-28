#version 330 core
out vec4 FragColor;

in vec3 ourColor;
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform sampler2D ourTexture;
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

// Luz puntual (hasta 4)
struct PointLight {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float constant;
    float linear;
    float quadratic;
    bool enabled;
};
#define MAX_POINT_LIGHTS 4
uniform PointLight pointLights[MAX_POINT_LIGHTS];
uniform int numPointLights;

// Propiedades del material
uniform float shininess = 32.0;

// Función para calcular luz direccional
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 baseColor) {
    if (!light.enabled) return vec3(0.0);
    
    vec3 lightDir = normalize(-light.direction);
    
    // Ambient - luz base para que todo sea visible
    vec3 ambient = light.ambient * baseColor;
    
    // Diffuse - iluminación simple basada en el ángulo
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * baseColor;
    
    return ambient + diffuse;
}

// Función para calcular luz puntual
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 baseColor) {
    if (!light.enabled) return vec3(0.0);
    
    vec3 lightDir = normalize(light.position - fragPos);
    
    // Ambient
    vec3 ambient = light.ambient * baseColor;
    
    // Diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * baseColor;
    
    // Atenuación
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    
    ambient *= attenuation;
    diffuse *= attenuation;
    
    return ambient + diffuse;
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
    
    // Luces puntuales
    for (int i = 0; i < numPointLights && i < MAX_POINT_LIGHTS; i++) {
        result += CalcPointLight(pointLights[i], norm, FragPos, baseColor);
    }
    
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
