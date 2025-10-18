#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform vec3 viewPos;
uniform bool fogEnabled = true;
uniform float fogDensity = 0.05;
uniform vec3 fogColor = vec3(0.7, 0.8, 0.9);

// Directional light
struct DirLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    bool enabled;
};
uniform DirLight dirLight;

// Simple pseudo-random function
float pseudoRandom(vec3 pos) {
    return fract(sin(dot(pos * 0.1, vec3(12.9898, 78.233, 45.164))) * 43758.5453);
}

float calculateFog(float distance) {
    if (!fogEnabled) return 1.0;
    return exp(-pow(distance * fogDensity, 1.6));
}

void main() {
    // Base dark green color for grass/terrain (oscuro, realista)
    vec3 baseColor = vec3(0.1, 0.35, 0.1);
    
    // Add slight random variation based on world position (micro-texturas)
    float variation = pseudoRandom(FragPos) * 0.3;
    baseColor += vec3(variation * 0.05, variation * 0.1, variation * 0.05);
    
    vec3 normal = normalize(Normal);
    vec3 lightDir = normalize(-dirLight.direction);
    
    // Lighting based on normal orientation
    float diff = max(dot(normal, lightDir), 0.0);
    
    // Ambient: siempre algo de luz
    vec3 ambient = dirLight.ambient * baseColor * 0.4;
    
    // Diffuse: varía mucho según orientación
    vec3 diffuse = dirLight.diffuse * diff * baseColor;
    
    // Hemi-light effect (cielo ilumina desde arriba)
    float hemilighting = 0.5 + 0.5 * normal.y;
    vec3 skyLight = vec3(0.3, 0.35, 0.4) * baseColor * hemilighting * 0.3;
    
    vec3 result = ambient + diffuse + skyLight;
    
    float distance = length(FragPos - viewPos);
    float fog_factor = calculateFog(distance);
    
    vec3 final_color = mix(fogColor, result, fog_factor);
    FragColor = vec4(final_color, 1.0);
}
