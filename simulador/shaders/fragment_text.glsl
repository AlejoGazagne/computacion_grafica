#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D textAtlas;
uniform vec4 textColor = vec4(1.0, 1.0, 1.0, 1.0);

void main() {
    float alpha = texture(textAtlas, TexCoords).r;
    FragColor = textColor * vec4(1.0, 1.0, 1.0, alpha);
}
