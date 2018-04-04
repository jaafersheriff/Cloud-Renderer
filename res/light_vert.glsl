#version 330 core

in vec3 vertPos;

uniform mat4 L;
uniform mat4 M;

void main() {
    gl_Position = L * M * vec4(vertPos, 1.0);
}