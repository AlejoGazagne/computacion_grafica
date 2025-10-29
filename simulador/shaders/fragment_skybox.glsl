#version 330 core
out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube skybox;

// Niebla
uniform bool fogEnabled = true;
uniform float fogDensity = 0.0001;         // misma magnitud que el terreno
uniform vec3 fogColor = vec3(0.85, 0.90, 0.95);

// Controles específicos del skybox para mapear densidad a ángulo de horizonte
uniform float skyFogScale = 15000.0;       // escala para mapear densidad a mezcla visible
uniform float skyFogMax = 0.95;            // tope de niebla en el horizonte
uniform float skyFogExponent = 1.4;        // curva: >1 enfatiza el horizonte

void main() {
    vec3 dir = normalize(TexCoords);
    vec4 skyboxColor = texture(skybox, TexCoords);

    if (fogEnabled) {
        // 0 arriba/abajo, 1 en el horizonte
        float horizon = 1.0 - abs(dir.y);
        // curva para ajustar cómo crece hacia el horizonte
        float curved = pow(horizon, skyFogExponent);
        // mapear densidad global a una mezcla visible en el skybox
        float fogAmount = clamp(curved * fogDensity * skyFogScale, 0.0, skyFogMax);
        FragColor = vec4(mix(skyboxColor.rgb, fogColor, fogAmount), skyboxColor.a);
    } else {
        FragColor = skyboxColor;
    }
}
