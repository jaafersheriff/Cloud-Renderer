#version 330 core

in vec4 worldPos;

uniform vec3 lightPos;
uniform bool isOutline;
uniform float visibility;

out vec4 color;

void main() {
    if (isOutline) {
        color = vec4(1, 0, 0, 1);
        return;
    }

    color = vec4(visibility);
}