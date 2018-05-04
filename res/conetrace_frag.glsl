#version 440 core

in vec3 fragPos;
in vec2 fragTex;

uniform vec3 center;
uniform float scale;

uniform int voxelDim;
uniform vec2 xBounds;
uniform vec2 yBounds;
uniform vec2 zBounds;
uniform vec3 lightPos;

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

vec3 traceCone(sampler3D voxelTexture, vec3 position, vec3 direction, int steps, float bias, float coneAngle, float coneHeight) {
    direction = normalize(direction);
    direction.z = -direction.z;
    direction /= voxelDim;
    vec3 start = position / voxelDim + bias * direction;

    vec3 color = vec3(0);
    float contrib = 1.f/steps;

    for (int i = 1; i <= steps; i++) {
        float coneRadius = coneHeight * tan(coneAngle / 2.f);
        float lod = log2(max(1.f, 2.f * coneRadius));
        vec4 sampleColor = textureLod(voxelTexture, start + coneHeight * direction, lod + vctLodOffset);
        color += sampleColor.rgb * i/steps; // * (i * contrib);
        coneHeight += coneRadius;
    }
    return color;
}

void main() {
    float radius = scale/2;

    /* Spherical distance - 1 at center of billboard, 0 at edges */
    float sphereContrib = (distance(center, fragPos)/radius);
    sphereContrib = sqrt(max(0, 1 - sphereContrib * sphereContrib));

    vec3 voxelPosition = calculateVoxelLerp(fragPos);
    vec3 dir = lightPos - fragPos; 
    vec3 indirect = traceCone(volumeTexture, voxelPosition, dir, vctSteps, vctBias, vctConeAngle, vctConeInitialHeight);
    color = vec4(indirect, sphereContrib);
}
