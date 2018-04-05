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
    // Radius of circle contained within quad 
    float radius = scale/2;

    // Calculate the distance ratio between the center of the circle and
    // the current fragment. This value is 1 for fragments at the center of the 
    // circle and 0 for fragments at the edge of the circle
    float distR = (distance(center, fragPos)/radius) * -1 + 1;

    color = vec4(distR);
    if(voxelize) {
        // Store data in the volume starting from the billboard and moving
        // towards its normal in increments of 0.2 until reaching the edge of
        // the sphere
        for(float j = 0; j < radius * distR; j += 0.2f) {
            imageStore(volume, voxelIndex(fragPos + normal * j), vec4(1, 0, 0, 0));
        }
    }
}
