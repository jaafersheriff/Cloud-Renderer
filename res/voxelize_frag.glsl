#version 440 core

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

layout(binding=1, rgba32f) uniform image3D volume;

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

    if(voxelize) {
        for(float j = 0; j < radius * distR; j += normalStep) {
            ivec3 voxelIndex = calculateVoxelIndex(fragPos + normal * j);
            vec4 voxelData = imageLoad(volume, voxelIndex) + vec4(0, 0, 0, visibilityContrib);
            imageStore(volume, voxelIndex, voxelData);
        }
    }
}
