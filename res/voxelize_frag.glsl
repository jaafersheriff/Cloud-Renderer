#version 440 core

#extension GL_NV_gpu_shader5: enable
#extension GL_NV_shader_atomic_float: enable
#extension GL_NV_shader_atomic_fp16_vector: enable

in vec3 fragPos;
in vec2 fragTex;

uniform vec2 xBounds;
uniform vec2 yBounds;
uniform vec2 zBounds;
uniform int voxelSize;
uniform bool voxelize;

uniform vec3 center;
uniform float scale;

uniform vec3 normal;

uniform float normalStep;
uniform float visibilityContrib;

uniform sampler2D lightMap;
uniform bool useLightMap;

layout(binding=1, rgba16f) uniform image3D volume;

out vec4 color;

/* Linear map from aribtray box(?) in world space to 3D volume 
 * Voxel indices: [0, voxelSize - 1] */
ivec3 calculateVoxelIndex(vec3 pos) {
    float rangeX = xBounds.y - xBounds.x;
    float rangeY = yBounds.y - yBounds.x;
    float rangeZ = zBounds.y - zBounds.x;

	float x = voxelSize * ((pos.x - xBounds.x) / rangeX);
	float y = voxelSize * ((pos.y - yBounds.x) / rangeY);
	float z = voxelSize * ((pos.z - zBounds.x) / rangeZ);

	return ivec3(x, y, z);
}

void main() {
    float radius = scale/2;

    // 1 at center of billboard, 0 at edges
    float distR = 1 - (distance(center, fragPos)/radius);
    color = vec4(distR);

    if (useLightMap) {
        vec4 worldPos = texture(lightMap, fragTex);
        ivec3 voxelIndex = calculateVoxelIndex(worldPos.xyz);
        imageStore(volume, voxelIndex, vec4(1, 1, 1, 1));
    }

    if(voxelize) {
        for(float j = 0; j < radius * distR; j += normalStep) {
            ivec3 voxelIndex = calculateVoxelIndex(fragPos + normal * j);
            /* Light voxelize - denote that this voxel has been voxelized by light */
            imageStore(volume, voxelIndex, vec4(0, 0, 0, 1));
// #if GL_NV_shader_atomic_fp16_vector
//             imageAtomicAdd(volume, voxelIndex, f16vec4(0, 0, 0, visibilityContrib));
// #else
//             vec4 voxelData = imageLoad(volume, voxelIndex) + vec4(0, 0, 0, visibilityContrib);
//             imageStore(volume, voxelIndex, voxelData);
// #endif
        }
    }
}
