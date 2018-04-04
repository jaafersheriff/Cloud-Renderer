#version 330 core

layout(location = 0) in vec4 vertPos;
layout(location = 1) in vec3 vertNor;

uniform mat4 P;
uniform mat4 M;
uniform mat4 V;

out vec4 worldPos;
out vec3 fragNormal;

void main() {
    worldPos = M * vertPos;
    gl_Position = P * V * worldPos;

    fragNormal = vec3(M * vec4(normalize(vertNor), 0.0));
}