#version 450 core

in vec3 fragPos;
in vec2 fragTex;

layout(binding=2, rgba16f) uniform image3D volume;

ivec3 voxelIndex(vec3 pos) {
	const float minx = -20, maxx = 20,
	miny = -2, maxy = 15,
	minz = -12, maxz = 12;
	const int voxelDim = 128;

	float rangex = maxx - minx;
	float rangey = maxy - miny;
	float rangez = maxz - minz;

	float x = voxelDim * ((pos.x - minx) / rangex);
	float y = voxelDim * ((pos.y - miny) / rangey);
	float z = voxelDim * ((pos.z - minz) / rangez);

	return ivec3(x, y, z);
}

void main() {
    vec4 color = vec4(1, 0, 0, 1);
    ivec3 i = voxelIndex(fragPos);
    imageStore(volume, i, color);
}
