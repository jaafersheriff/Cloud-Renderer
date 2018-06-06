#version 440 core

#extension GL_NV_gpu_shader5: enable
#extension GL_NV_shader_atomic_float: enable
#extension GL_NV_shader_atomic_fp16_vector: enable

#define FLT_MAX 3.402823466e+38

in vec3 fragPos;
in vec3 fragNor;

uniform vec3 center;
uniform float scale;

uniform vec3 lightPos;
uniform float maxDist;

layout(binding=0, rgba16f) uniform image3D volume;
uniform int voxelDim;
uniform vec2 xBounds;
uniform vec2 yBounds;
uniform vec2 zBounds;
uniform float stepSize;

out vec4 color;

/* Linear map from aribtray box(?) in world space to 3D volume 
 * Voxel indices: [0, dimension - 1] */
vec3 calculateVoxelLerp(vec3 pos) {
    float rangeX = xBounds.y - xBounds.x;
    float rangeY = yBounds.y - yBounds.x;
    float rangeZ = zBounds.y - zBounds.x;

    float x = voxelDim * ((pos.x - xBounds.x) / rangeX);
    float y = voxelDim * ((pos.y - yBounds.x) / rangeY);
    float z = voxelDim * ((pos.z - zBounds.x) / rangeZ);

	return vec3(x, y, z);
}

ivec3 calculateVoxelIndex(vec3 pos) {
	return ivec3(calculateVoxelLerp(pos));
}

void main() {
    float radius = scale;

    /* Spherical distance - 1 at center of billboard, 0 at edges */
    float sphereContrib = (distance(center, fragPos)/radius);
    sphereContrib = sqrt(max(0, 1 - sphereContrib * sphereContrib));

    if (sphereContrib < 0.01f) {
        discard;
    }

    /* Write to volume in spherical shape from billboard to light source */
    vec3 dir = normalize(fragNor); 
    float dist = radius * sphereContrib;
    vec3 start = fragPos - dir * dist;
    for(float i = 0; i < 2*dist; i += stepSize) {
        vec3 worldPos = start + dir * i;
        ivec3 voxelIndex = calculateVoxelIndex(worldPos);
        // imageAtomicAdd(volume, voxelIndex, f16vec4(1, 0, 0, 1));
        imageStore(volume, voxelIndex, vec4(0, 0, 0, 1));
    }

    /* Write nearest voxel position to position FBO */
    vec3 worldPos = fragPos + dir * dist;
    color = vec4(worldPos, 1);
    gl_FragDepth = distance(lightPos, worldPos) / maxDist;
}
