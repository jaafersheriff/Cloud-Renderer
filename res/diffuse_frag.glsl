#version 330 core

in vec4 worldPos;
in vec3 fragNormal;

uniform vec3 lightPos;
uniform bool isOutline;

out vec4 color;

void main() {
    if (isOutline) {
        color = vec4(1, 1, 1, 1);
        return;
    }

    vec3 L = normalize(lightPos - worldPos.xyz);
    vec3 N = normalize(fragNormal);

    /* Diffuse */
    float diffuseContrib = clamp(dot(L, N), 0.0, 1.0);

    color = vec4(vec3(diffuseContrib), 1.0);
}