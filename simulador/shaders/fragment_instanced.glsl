#version 330 core
out vec4 FragColor;

in vec3 ourColor;
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform sampler2D ourTexture;
uniform bool useTexture;
uniform vec3 viewPos;

// Fog
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

uniform float shininess = 32.0;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 baseColor) {
    if (!light.enabled) return vec3(0.0);
    
    vec3 lightDir = normalize(-light.direction);
    float diff = max(dot(normal, lightDir), 0.0);
    
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    
    vec3 ambient = light.ambient * baseColor;
    vec3 diffuse = light.diffuse * diff * baseColor;
    vec3 specular = light.specular * spec;
    
    return ambient + diffuse + specular;
}

float calculateFog(float distance) {
    if (!fogEnabled) return 1.0;
    return exp(-pow(distance * fogDensity, 1.6));
}

void main() {
    vec3 normal = normalize(Normal);
    
    vec3 baseColor = ourColor;
    if (useTexture) {
        baseColor = texture(ourTexture, TexCoords).rgb;
    }
    
    vec3 result = CalcDirLight(dirLight, normal, baseColor);
    
    float distance = length(FragPos - viewPos);
    float fog_factor = calculateFog(distance);
    
    vec3 final_color = mix(fogColor, result, fog_factor);
    FragColor = vec4(final_color, 1.0);
}
