#version 330 core

in vec2 fragTex;

uniform sampler2D diffuseTex;

out vec4 color;

void main() {
    color = vec4(1.f, 0.f, 0.f, 1.f);
    // color = texture(diffuseTex, fragTex);
}