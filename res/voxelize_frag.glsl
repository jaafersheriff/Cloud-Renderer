#version 440 core

in vec4 worldPos;
in vec3 fragPos;
in vec2 fragTex;

uniform vec2 xBounds;
uniform vec2 yBounds;
uniform vec2 zBounds;
uniform int voxelSize;

layout(binding=1, rgba16f) uniform image3D volume;

out vec4 color;

ivec3 voxelIndex(vec3 pos) {
    float rangeX = xBounds.y - xBounds.x;
    float rangeY = yBounds.y - yBounds.x;
    float rangeZ = zBounds.y - zBounds.x;

	float x = voxelSize * ((pos.x - xBounds.x) / rangeX);
	float y = voxelSize * ((pos.y - yBounds.x) / rangeY);
	float z = voxelSize * (1 - (pos.z - zBounds.x) / rangeZ);

	return ivec3(x, y, z);
}

void main() {
    color = vec4(1, 0, 0, 1);
    ivec3 i = voxelIndex(worldPos.xyz);
    imageStore(volume, i, worldPos);
}
