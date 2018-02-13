#version 450 core

in vec3 fragPos;
in vec2 fragTex;

uniform vec2 xBounds;
uniform vec2 yBounds;
uniform vec2 zBounds;
uniform int voxelSize;

layout(binding=1, rgba16f) uniform image3D volume;

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
    vec4 color = vec4(1, 0, 0, 1);
    ivec3 i = voxelIndex(fragPos);
    imageStore(volume, i, color);
}
