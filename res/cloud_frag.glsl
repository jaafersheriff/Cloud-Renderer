#version 330 core

in vec3 fragPos;
in vec2 fragTex;

uniform vec3 lightPos;
uniform sampler2D diffuseTex;

out vec4 color;

#define PI 1.57079632679

void main() {
    vec3 L = normalize(lightPos - fragPos);
    vec4 diffuseTexColor = texture(diffuseTex, fragTex);
    vec2 stc= PI * (fragTex - vec2(0.5, 0.5));

    vec3 normal = vec3(sin(stc.x), sin(stc.y), cos(stc.x) * cos(stc.y));

    float d = clamp(dot(L, normal), 0, 1);
    // d = pow(1.7);

    color = vec4(vec3(d), 1.0);
}