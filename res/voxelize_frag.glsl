#version 440 core

#extension GL_NV_gpu_shader5: enable
#extension GL_NV_shader_atomic_float: enable
#extension GL_NV_shader_atomic_fp16_vector: enable

#define FLT_MAX 3.402823466e+38

in vec3 fragPos;
in vec2 fragTex;

uniform vec3 center;
uniform float scale;

uniform int voxelizeStage;

layout(binding=0, rgba16f) uniform image3D volume;
uniform vec2 xBounds;
uniform vec2 yBounds;
uniform vec2 zBounds;
uniform int dimension;
uniform vec3 voxelSize;
uniform float steps;
uniform vec3 lightPos;

layout(binding=1, rgba32f) uniform image2D positionMap;
uniform int mapWidth;
uniform int mapHeight;

uniform sampler3D volumeTexture;
uniform int vctSteps;
uniform float vctBias;
uniform float vctConeAngle;
uniform float vctConeInitialHeight;
uniform float vctLodOffset;
uniform vec3 camPos;

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

vec4 traceCone(sampler3D voxelTexture, vec3 position, vec3 direction, int steps, float bias, float coneAngle, float coneHeight) {
    direction = normalize(direction);
    direction.z = -direction.z;
    direction /= dimension;
    vec3 start = position + bias * direction;

    vec4 color = vec4(0);

    for (int i = 0; i < steps && color.a < 0.95; i++) {
        float coneRadius = coneHeight * tan(coneAngle / 2.0);
        float lod = log2(max(1.0, 2 * coneRadius));
        vec4 sampleColor = textureLod(voxelTexture, normalize(start + coneHeight * direction), lod + vctLodOffset);
        float a = 1 - color.a;
        color.xyz += sampleColor.rgb * a;
        color.a += a * sampleColor.a;
        coneHeight += coneRadius;
    }

    return color;
}

void main() {
    float radius = scale/2;

    /* Spherical distance - 1 at center of billboard, 0 at edges */
    float distR = (distance(center, fragPos)/radius);
    distR = sqrt(max(0, 1 - distR * distR));
    color = vec4(distR);

    ivec2 texCoords = ivec2(fragTex.x * mapWidth, fragTex.y * mapHeight);
    if (length(fragTex*2-1) > 1) {
        return;
    }
 
    /* First voxelize - set blacks voxels in a sphere, write to position map */
    if (voxelizeStage == 1) {
        vec3 normal = normalize(lightPos - center);
        float normalScale = radius * distR;
        vec3 nearestPos = vec3(FLT_MAX, FLT_MAX, FLT_MAX);
        for(float j = -normalScale; j <= normalScale; j += steps) {
            vec3 worldPos = fragPos + (normal * j);
            ivec3 voxelIndex = calculateVoxelIndex(worldPos);
            imageAtomicAdd(volume, voxelIndex, f16vec4(0, 0, 0, 1));

            if (distance(worldPos, lightPos) < distance(nearestPos, lightPos)) {
                nearestPos = worldPos;
            }
        }
        if (nearestPos.x != FLT_MAX) {
            imageStore(positionMap, texCoords, vec4(nearestPos, 1.0));
        }
    }
    /* Second voxelize - set white voxels using position map */
    else if (voxelizeStage == 2) {
        vec4 worldPos = imageLoad(positionMap, texCoords);
        if (worldPos.a > 0) {
            ivec3 voxelIndex = calculateVoxelIndex(worldPos.xyz);
            imageStore(volume, voxelIndex, vec4(1, 1, 1, 1));
        }
    }
    /* Cone trace */
    else if (voxelizeStage == 3) {
        vec3 coneDirs[4] = vec3[] (
            vec3( 0.707, 0.707, 0),
            vec3( 0, 0.707, 0.707),
            vec3(-0.707, 0.707, 0),
            vec3( 0, 0.707, -0.707)
        );
        float coneWeights[4] = float[](0.25, 0.25, 0.25, 0.25);
        
        /* Start at sphere surface */
        vec3 normal = normalize(camPos - center);
        vec3 worldPos = fragPos + (normal * radius * distR);
        vec3 voxelPosition = vec3(calculateVoxelIndex(worldPos));
        vec4 indirect = vec4(0);
        for (int i = 0; i < 4; i++) {
            // TODO : rotate cones 
            vec3 dir = normalize(coneDirs[i]);
            indirect += coneWeights[i] * traceCone(volumeTexture, voxelPosition, dir, vctSteps, vctBias, vctConeAngle, vctConeInitialHeight);
        }
        color = indirect;
    }

}
