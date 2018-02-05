#version 330 core
layout(location = 0) in vec3 vertPos;
layout(location = 0) in vec2 vertTex;

uniform mat4 P;
uniform mat4 M;
uniform mat4 V;

out vec3 fragPos;
out vec2 fragTex;

void main() {
    fragPos = vertPos;
    fragTex = vertTex;
    gl_Position = P * V * M * vec4(vertPos, 1.0);
}