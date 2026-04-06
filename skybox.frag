#version 330 core
out vec4 FragColor;
in vec3 TexCoords;

uniform samplerCube skybox; // Observ? tipul samplerCube pentru 6 imagini

void main() {    
    FragColor = texture(skybox, TexCoords);
}