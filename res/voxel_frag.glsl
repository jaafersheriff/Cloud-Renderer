#version 330 core

in vec4 voxData;

uniform bool isOutline;
uniform float alpha;

out vec4 color;

void main() {
    if (isOutline) {
        color = vec4(0.5, 0.5, 0.5, 1);
        return;
    }

    color = voxData;
    color.a = alpha;
}