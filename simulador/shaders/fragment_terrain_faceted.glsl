#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform vec3 viewPos;
uniform bool fogEnabled;
uniform float fogDensity;
uniform vec3 fogColor;

struct DirLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    bool enabled;
};
uniform DirLight dirLight;

float calculateFog(float distance) {
    if (!fogEnabled) return 1.0;
    return exp(-pow(distance * fogDensity, 1.6));
}

void main() {
    vec3 baseColor = vec3(0.1, 0.35, 0.1);
    
    vec3 normal = normalize(Normal);
    vec3 lightDir = normalize(-dirLight.direction);
    
    float diff = max(dot(normal, lightDir), 0.0);
    
    vec3 ambient = dirLight.ambient * baseColor * 0.4;
    vec3 diffuse = dirLight.diffuse * diff * baseColor;
    
    vec3 result = ambient + diffuse;
    
    float distance = length(FragPos - viewPos);
    float fog_factor = calculateFog(distance);
    vec3 final_color = mix(fogColor, result, fog_factor);
    
    FragColor = vec4(final_color, 1.0);
}
