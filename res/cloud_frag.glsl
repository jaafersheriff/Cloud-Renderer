#version 330 core

in vec3 fragPos;
in vec2 fragTex;

uniform mat4 Vi;

uniform sampler2D diffuseTex;
uniform sampler2D normalTex;

uniform vec3 lightPos;

out vec4 color;

#define halfPI 1.57079632679

void main() {
    vec3 L = normalize(lightPos - fragPos);

    vec4 diffuseTexColor = texture(diffuseTex, fragTex);
    vec4 normalTexColor = texture(normalTex, fragTex);
    vec2 stc = halfPI * (fragTex - vec2(0.5, 0.5));

    // if (length(stc) > 1.0) 
    //     discard;

    vec3 normal = vec3(sin(stc.x), sin(stc.y), cos(stc.x) * cos(stc.y));
    vec3 tangent = vec3(cos(stc.x), 0, sin(stc.x));
    vec3 binormal = cross(normal, tangent);

    // Calculate the normal from the data in the bump map
    vec3 bumpNorm = (normalTexColor.xyz * 2.0) - 1.0;
    vec3 bumpNormal = (bumpNorm.x * tangent) + (bumpNorm.y * binormal) + (bumpNorm.z * normal);
    normal = (Vi * vec4(bumpNormal, 0.0)).xyz;

    float d = clamp(dot(L, normal), 0, 1);
    // d = pow(d, 1.7);

    color.rgb = d * diffuseTexColor.rgb;
    color.a = diffuseTexColor.a;

    // sun viz - TODO : Remove
    float distR = 1 - length(fragTex*2-1);
    color = vec4(distR, distR, 0, distR);
}