#version 440 core

in vec3 fragPos;

layout(binding=0, r8) uniform image3D volume;
uniform int voxelDim;
uniform vec2 xBounds;
uniform vec2 yBounds;
uniform vec2 zBounds;
uniform float stepSize;

layout(binding=1, rgba32f) uniform image2D positionMap;

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
    /* Read from position map */
    vec4 worldPos = imageLoad(positionMap, ivec2(gl_FragCoord.xy));
    /* If this voxel is active (is already either black or white)
     * Set it to white */
    if (worldPos.a > 0) {
        vec4 col = vec4(1);
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
