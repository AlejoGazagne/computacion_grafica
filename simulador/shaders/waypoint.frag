#version 460 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;

uniform vec3 waypointColor;
uniform float waypointAlpha;
uniform bool isActive;

void main()
{
    // Iluminación simple
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(vec3(0.5, 1.0, 0.3));
    float diff = max(dot(norm, lightDir), 0.0);
    
    // Ambiente
    float ambient = 0.4;
    
    // Efecto de brillo si es activo
    float glow = isActive ? 0.4 : 0.0;
    
    vec3 result = waypointColor * (ambient + diff + glow);
    FragColor = vec4(result, waypointAlpha);
}
