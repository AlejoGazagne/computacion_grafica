#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

// Instance attributes
layout (location = 5) in vec3 aInstancePos;
layout (location = 6) in vec3 aInstanceScale;
layout (location = 7) in float aInstanceRotY;
layout (location = 8) in float aInstanceBillboard;

out vec3 ourColor;
out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

uniform mat4 view;
uniform mat4 projection;
uniform vec3 cameraPos;
uniform vec3 cameraRight;
uniform vec3 cameraUp;

void main() {
    vec3 world_pos = aPos * aInstanceScale;
    
    // Si es billboard, orientar hacia la cámara
    if (aInstanceBillboard > 0.5) {
        // Billboard cilíndrico: mantener base en suelo, rotar solo alrededor de Y
        // Construir matriz de billboard (sin rotación Y del mesh)
        vec3 to_camera = normalize(cameraPos - aInstancePos);
        vec3 right = normalize(cross(vec3(0, 1, 0), to_camera));
        vec3 up = vec3(0, 1, 0);
        
        // Aplicar scaling y billboard
        world_pos = right * (aPos.x * aInstanceScale.x) +
                   up * (aPos.y * aInstanceScale.y) +
                   cross(up, right) * (aPos.z * aInstanceScale.z);
    } else {
        // Rotación Y normal del mesh
        float cos_ry = cos(aInstanceRotY);
        float sin_ry = sin(aInstanceRotY);
        
        vec3 rotated = vec3(
            world_pos.x * cos_ry - world_pos.z * sin_ry,
            world_pos.y,
            world_pos.x * sin_ry + world_pos.z * cos_ry
        );
        
        world_pos = rotated;
    }
    
    // Agregar posición de instancia
    world_pos += aInstancePos;
    
    // Normal (para mallas no-billboard; para billboard, calcular en fragment)
    Normal = aNormal;
    TexCoords = aTexCoords;
    FragPos = world_pos;
    
    gl_Position = projection * view * vec4(world_pos, 1.0);
    ourColor = vec3(0.5, 0.5, 0.5);
}
