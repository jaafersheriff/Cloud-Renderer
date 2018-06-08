#version 330 core

in vec3 fragPos;

uniform vec3 center;

uniform vec3 innerColor;
uniform vec3 outerColor;
uniform float innerRadius;
uniform float outerRadius;

out vec4 color;

void main() {
    float dist = distance(center, fragPos);

    /* Inner circle */
    if (dist < innerRadius) {
        color = vec4(innerColor, 1.0);
    }
    /* Outer circle */
    else {
        float scale = (dist - innerRadius) / (outerRadius - innerRadius);
        if (scale > 0.99f) {
            discard;
        }
        color = vec4(outerColor * scale + innerColor * (1 - scale), 1 - scale);
    }
}