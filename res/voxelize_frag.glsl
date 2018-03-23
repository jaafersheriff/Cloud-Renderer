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

layout(binding=1, rgba32f) uniform image3D volume;

out vec4 color;

/* Linear map from aribtray box(?) in world space to 3D volume 
 * Voxel indices: [0, voxelSize - 1] */
ivec3 voxelIndex(vec3 pos) {
    float rangeX = xBounds.y - xBounds.x;
    float rangeY = yBounds.y - yBounds.x;
    float rangeZ = zBounds.y - zBounds.x;

	float x = voxelSize * ((pos.x - xBounds.x) / rangeX);
	float y = voxelSize * ((pos.y - yBounds.x) / rangeY);
	float z = voxelSize * ((pos.z - zBounds.x) / rangeZ);

	return ivec3(x, y, z);
}

void main() {
    float dist = (distance(center, fragPos)/scale) * -1 + 1;
    color = vec4(dist);
    if(voxelize) {
        imageStore(volume, voxelIndex(fragPos), vec4(1, 0, 0, 0));

        float radius = scale/2;
        ivec3 i = voxelIndex(fragPos);
        for(float j = 0; j < radius * dist; j += 0.2f) {
            imageStore(volume, voxelIndex(fragPos + normal * j), vec4(1, 0, 0, 0));
        }
    }
}
