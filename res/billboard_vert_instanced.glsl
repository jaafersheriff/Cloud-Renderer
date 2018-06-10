#version 330 core

layout(location = 0) in vec3 vertPos;
layout(location = 1) in vec3 vertNor;
layout(location = 2) in vec3 boardPosition;
layout(location = 3) in float boardScale;

uniform mat4 P;
uniform mat4 V;
uniform mat4 Vi;

uniform vec3 volumePosition;

out vec3 fragPos;
out vec3 fragNor;
out vec2 fragTex;
flat out vec3 center;
flat out float scale;

void main() {
    vec3 finalPos = volumePosition + boardPosition;
    mat4 M = mat4(1.f);
    M[0][0] = boardScale; 
    M[1][1] = boardScale; 
    M[2][2] = boardScale; 
    M[3][0] = finalPos.x;
    M[3][1] = finalPos.y;
    M[3][2] = finalPos.z;
    M[3][3] = 1.f;

    vec4 worldPos = M * Vi * vec4(vertPos, 1.0);
    gl_Position = P * V * worldPos;
    fragPos = worldPos.xyz;
    fragNor = mat3(transpose(inverse(M * Vi))) * vertNor;
    fragTex = (vertPos.xy + 1) / 2.f;
    center = finalPos;
    scale = boardScale;
}