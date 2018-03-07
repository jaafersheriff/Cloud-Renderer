#version 330 core

in vec4 worldPos;
in vec3 fragNormal;

uniform vec3 lightPos;

out vec4 color;

void main() {
    vec3 lightDir = lightPos - worldPos.xyz;
    vec3 L = normalize(lightDir);
    vec3 N = normalize(fragNormal);

    /* Diffuse */
    float diffuseContrib = clamp(dot(L, N), 0.0, 1.0);

    color = vec4(vec3(diffuseContrib), 1.0);
   // color = vec4(0.3, 0.3, 0.3, 1.0);
}