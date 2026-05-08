#version 330 core
out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube skybox;

void main() {
    vec3 coords = TexCoords;
    coords.y = -coords.y;
    FragColor = texture(skybox, coords);
}
