#version 330 core

in vec3 vertPos;

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

out vec4 worldPos;

void main() {
    worldPos = M * vec4(vertPos, 1.0);
    gl_Position = P * V * worldPos;
}