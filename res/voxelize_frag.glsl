#version 440 core

#extension GL_NV_gpu_shader5: enable
#extension GL_NV_shader_atomic_float: enable
#extension GL_NV_shader_atomic_fp16_vector: enable

#define FLT_MAX 3.402823466e+38

in vec3 fragPos;
in vec2 fragTex;

uniform vec3 lightPos;

uniform vec3 center;
uniform float scale;

layout(binding=0, rgba16f) uniform image3D volume;
uniform vec2 xBounds;
uniform vec2 yBounds;
uniform vec2 zBounds;
uniform int dimension;
uniform bool voxelize;

uniform float steps;

out vec4 color;

/* Linear map from aribtray box(?) in world space to 3D volume 
 * Voxel indices: [0, dimension - 1] */
ivec3 calculateVoxelIndex(vec3 pos) {
    float rangeX = xBounds.y - xBounds.x;
    float rangeY = yBounds.y - yBounds.x;
    float rangeZ = zBounds.y - zBounds.x;

	float x = dimension * ((pos.x - xBounds.x) / rangeX);
	float y = dimension * ((pos.y - yBounds.x) / rangeY);
	float z = dimension * ((pos.z - zBounds.x) / rangeZ);

	return ivec3(x, y, z);
}

void main() {
    float radius = scale/2;
    vec3 normal = normalize(lightPos - center);

    /* Spherical distance - 1 at center of billboard, 0 at edges */
    float distR = (distance(center, fragPos)/radius);
    distR = sqrt(max(0, 1 - distR * distR));
    color = vec4(distR);

    if (length(fragTex*2-1) > 1) {
        return;
    }

    float normalScale = radius * distR;
    for(float j = -normalScale; j < normalScale; j += normalScale/steps) {
        vec3 worldPos = fragPos + (normal * j);
        ivec3 voxelIndex = calculateVoxelIndex(worldPos);
        imageAtomicAdd(volume, voxelIndex, f16vec4(0, 0, 0, 1));
    }
    vec3 nPos = fragPos + normal * normalScale;
    ivec3 nvoxel = calculateVoxelIndex(nPos);
    imageStore(volume, nvoxel, vec4(1, 1, 1, 1));
}
