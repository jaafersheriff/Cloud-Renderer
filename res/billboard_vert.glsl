#version 330 core

layout(location = 0) in vec3 vertPos;
layout(location = 1) in vec3 vertNor;

uniform mat4 P;
uniform mat4 M;
uniform mat3 N;
uniform mat4 V;
uniform mat4 Vi;

out vec3 fragPos;
out vec3 fragNor;
out vec2 fragTex;

void main() {
    vec4 worldPos = M * Vi * vec4(vertPos, 1.0);
    gl_Position = P * V * worldPos;
    fragPos = worldPos.xyz;
    fragNor = N * vertNor; 
    fragTex = (vertPos.xy + 1) / 2.f;
}