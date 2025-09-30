#version 330 core
out vec4 FragColor;

in vec3 ourColor;
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform sampler2D ourTexture;
uniform bool useTexture;

void main() {
    // Iluminación básica
    vec3 lightPos = vec3(5.0, 5.0, 5.0);
    vec3 lightColor = vec3(1.0, 1.0, 1.0);
    
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    
    // Ambient
    float ambientStrength = 0.3;
    vec3 ambient = ambientStrength * lightColor;
    
    // Diffuse
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    
    vec3 lighting = ambient + diffuse;
    
    if (useTexture) {
        vec4 texColor = texture(ourTexture, TexCoords);
        FragColor = vec4(lighting * texColor.rgb, texColor.a);
    } else {
        FragColor = vec4(lighting * ourColor, 1.0);
    }
}
