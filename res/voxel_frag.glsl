#version 330 core

in float voxData;

uniform bool isOutline;
uniform float alpha;

out vec4 color;

void main() {
    if (isOutline) {
        color = vec4(0.5, 0.5, 0.5, 1);
        return;
    }

    color.rgb = vec3(voxData);
    color.a = alpha;
}