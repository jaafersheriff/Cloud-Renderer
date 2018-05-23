#version 330 core

in vec3 fragPos;
in vec2 fragTex;

uniform sampler2D positionMap;

out vec4 color;

void main() {
    color = texture(positionMap, fragTex);
}