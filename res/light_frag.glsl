#version 330 core

in vec4 worldPos;

out vec4 color;

void main() {
    color = worldPos;
}