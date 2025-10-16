#version 330 core
out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube skybox;

// Niebla
uniform bool fogEnabled = true;
uniform float fogDensity = 0.05;
uniform vec3 fogColor = vec3(0.7, 0.8, 0.9);

void main() {    
    vec4 skyboxColor = texture(skybox, TexCoords);
    
    // Aplicar niebla basada en la altura (coordenada Y)
    // Más niebla cerca del horizonte (y cercano a 0), menos niebla arriba/abajo
    if (fogEnabled) {
        float horizonFactor = 1.0 - abs(normalize(TexCoords).y);  // 0 en arriba/abajo, 1 en horizonte
        float fogAmount = horizonFactor * 0.7;  // Máximo 70% de niebla en el horizonte
        FragColor = vec4(mix(skyboxColor.rgb, fogColor, fogAmount), skyboxColor.a);
    } else {
        FragColor = skyboxColor;
    }
}
