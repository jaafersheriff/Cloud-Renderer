#version 330 core

layout(location = 0) in vec3 vertPos;

uniform mat4 P;
uniform mat4 M;
uniform mat4 V;
uniform mat4 Vi;

out vec4 worldPos;
out vec3 fragPos;
out vec2 fragTex;

void main() {
    worldPos = M * Vi * vec4(vertPos, 1.0);
    gl_Position = P * V * worldPos;
    fragPos = worldPos.xyz;
    fragTex = (vertPos.xy + 1.0) / 2.0;
}