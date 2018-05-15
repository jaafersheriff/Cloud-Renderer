#version 440 core

#extension GL_NV_gpu_shader5: enable
#extension GL_NV_shader_atomic_float: enable
#extension GL_NV_shader_atomic_fp16_vector: enable

#define FLT_MAX 3.402823466e+38

in vec3 fragPos;
in vec3 fragNor;
in vec2 fragTex;

uniform vec3 center;
uniform float scale;

uniform int voxelizeStage;

layout(binding=0, rgba16f) uniform image3D volume;
uniform int voxelDim;
uniform vec2 xBounds;
uniform vec2 yBounds;
uniform vec2 zBounds;
uniform float stepSize;
uniform vec3 lightPos;

layout(binding=1, rgba32f) uniform image2D positionMap;
uniform int mapWidth;
uniform int mapHeight;

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
    float radius = scale/2;

    /* Spherical distance - 1 at center of billboard, 0 at edges */
    float sphereContrib = (distance(center, fragPos)/radius);
    sphereContrib = sqrt(max(0, 1 - sphereContrib * sphereContrib));
    color = vec4(sphereContrib);

    ivec2 texCoords = ivec2(fragTex.x * mapWidth, fragTex.y * mapHeight);
    /* First Voxelize */
    if (voxelizeStage == 1 && sphereContrib > 0.f) {
        vec3 dir = normalize(fragNor); 
        float dist = radius * sphereContrib;
        vec3 nearestPos = vec3(FLT_MAX, FLT_MAX, FLT_MAX);
        vec3 start = fragPos - dir * dist;
        /* Write to volume in spherical shape from billboard to light source */
        for(float i = 0; i < 2*dist; i += stepSize) {
            vec3 worldPos = start + dir * i;
            ivec3 voxelIndex = calculateVoxelIndex(worldPos);
            imageAtomicAdd(volume, voxelIndex, f16vec4(0, 0, 0, 1));

            /* Keep track of nearest voxel position */
            if (distance(worldPos, lightPos) < distance(nearestPos, lightPos)) {
                nearestPos = worldPos;
            }
        }
        /* Write nearest voxel position to position map */
        if (nearestPos.x != FLT_MAX) {
            imageStore(positionMap, texCoords, vec4(nearestPos, 1.0));
        }
    }
    /* Second Voxelize */
    else if (voxelizeStage == 2) {
        /* Read from position map */
        vec4 worldPos = imageLoad(positionMap, texCoords);
        /* If this voxel is active (is already either black or white)
         * Set it to white */
        if (worldPos.a > 0) {
            vec4 col = vec4(1.f);
            imageStore(volume, calculateVoxelIndex(worldPos.xyz), col);
            imageStore(volume, calculateVoxelIndex(worldPos.xyz + stepSize * normalize(vec3( 1,  1,  1))), col);
            imageStore(volume, calculateVoxelIndex(worldPos.xyz + stepSize * normalize(vec3( 1,  1, -1))), col);
            imageStore(volume, calculateVoxelIndex(worldPos.xyz + stepSize * normalize(vec3( 1, -1,  1))), col);
            imageStore(volume, calculateVoxelIndex(worldPos.xyz + stepSize * normalize(vec3( 1, -1, -1))), col);
            imageStore(volume, calculateVoxelIndex(worldPos.xyz + stepSize * normalize(vec3(-1,  1,  1))), col);
            imageStore(volume, calculateVoxelIndex(worldPos.xyz + stepSize * normalize(vec3(-1,  1, -1))), col);
            imageStore(volume, calculateVoxelIndex(worldPos.xyz + stepSize * normalize(vec3(-1, -1,  1))), col);
            imageStore(volume, calculateVoxelIndex(worldPos.xyz + stepSize * normalize(vec3(-1, -1, -1))), col);
        }
    }
}
