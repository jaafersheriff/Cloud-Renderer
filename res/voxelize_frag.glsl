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
uniform int voxelDim;
uniform vec2 xBounds;
uniform vec2 yBounds;
uniform vec2 zBounds;
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

mat4 rotationMatrix(vec3 axis, float angle)
{
    axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;
    
    return mat4(
        oc*axis.x*axis.x+c,        oc*axis.x*axis.y-axis.z*s, oc*axis.z*axis.x+axis.y*s, 0.0,
        oc*axis.x*axis.y+axis.z*s, oc*axis.y*axis.y+c,        oc*axis.y*axis.z-axis.x*s, 0.0,
        oc*axis.z*axis.x-axis.y*s, oc*axis.y*axis.z+axis.x*s, oc*axis.z*axis.z+c,        0.0,
        0.0,                       0.0,                       0.0,                       1.0);
}

vec3 traceCone(sampler3D voxelTexture, vec3 position, vec3 direction, int steps, float bias, float coneAngle, float coneHeight) {
    direction = normalize(direction);
    direction.z = -direction.z;
    direction /= voxelDim;
    vec3 start = position + bias * direction;

    vec3 color = vec3(0);
    float alpha = 0;

    for (int i = 0; i < steps && alpha < 0.95f; i++) {
        float coneRadius = coneHeight * tan(coneAngle / 2.f);
        float lod = log2(max(1.f, 2.f * coneRadius));
        vec4 sampleColor = textureLod(voxelTexture, normalize(start + coneHeight * direction), lod + vctLodOffset);
        float a = 1 - alpha;
        color += sampleColor.rgb * a;
        alpha += a * sampleColor.a;
        coneHeight += coneRadius;
    }

    return color;
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
        vec3 normal = normalize(lightPos - center);
        float normalScale = radius * sphereContrib;
        vec3 nearestPos = vec3(FLT_MAX, FLT_MAX, FLT_MAX);
        /* Write to volume in spherical shape from billboard to light source */
        for(float j = -normalScale; j <= normalScale; j += steps) {
            vec3 worldPos = fragPos + (normal * j);
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
            ivec3 voxelIndex = calculateVoxelIndex(worldPos.xyz);
            imageStore(volume, voxelIndex, vec4(1, 1, 1, 1));
        }
    }
    /* Cone Trace */
    else if (voxelizeStage == 3 && sphereContrib > 0.f) {
        /* Cone params */
        vec3 coneDirs[4] = vec3[] (
            vec3( 0.707, 0.707, 0),
            vec3( 0, 0.707, 0.707),
            vec3(-0.707, 0.707, 0),
            vec3( 0, 0.707, -0.707)
        );
        float coneWeights[4] = float[](0.25, 0.25, 0.25, 0.25);

        /* Used to rotate cones to face light source */
        vec3 direction = normalize(lightPos - center); // TODO : fragpos? 
        vec3 axis = cross(vec3(0, 1, 0), direction);
        float angle = acos(dot(vec3(0, 1, 0), direction));
        mat4 rotation = rotationMatrix(axis, angle);

        /* Interpolated position */
        vec3 voxelPosition = calculateVoxelLerp(fragPos);

        vec3 indirect = vec3(0);
        for (int i = 0; i < 4; i++) {
            /* Rotate cones to face light source */
            vec3 dir = normalize(vec3(vec4(coneDirs[i],1)*rotation));
            indirect += coneWeights[i] * traceCone(volumeTexture, voxelPosition, dir, vctSteps, vctBias, vctConeAngle, vctConeInitialHeight);
        }
        color = vec4(indirect, sphereContrib);
    }

}
