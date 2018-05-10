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
uniform vec3 camPos;

uniform sampler3D volumeTexture;
uniform int vctSteps;
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

vec3 traceCone(sampler3D voxelTexture, vec3 position, vec3 direction, int steps, float coneAngle, float coneHeight) {
    direction = normalize(direction);
    direction /= voxelDim;
    position /= voxelDim;
    // position = clamp(position, 0.f, 1.f);

    // vec4 c = textureLod(voxelTexture, position, 0);
    // if (c.a == 0.f) {
    //     return vec3(1,0,0);
    // }
    // else {
    //    return c.xyz;
    // }

    vec3 color = vec3(0);
    for (int i = 1; i <= steps; i++) {
        float coneRadius = coneHeight * tan(coneAngle / 2.f);
        float lod = log2(max(1.f, 2.f * coneRadius));
        vec4 sampleColor = textureLod(voxelTexture, position + coneHeight * direction, lod + vctLodOffset);
        color += sampleColor.rgb * float(i)/steps; // TODO : linear scaling
        coneHeight += coneRadius;
    }

    return color;
}

void main() {
    float radius = scale/2;

    /* Spherical distance - 1 at center of billboard, 0 at edges */
    float sphereContrib = (distance(center, fragPos)/radius);
    sphereContrib = sqrt(max(0, 1 - sphereContrib * sphereContrib));

    /* Start at voxel closest to camera */
    vec3 pos = fragPos + normalize(camPos - center) * radius * sphereContrib;
    vec3 voxelPosition = calculateVoxelLerp(pos);
    vec3 dir = lightPos - pos; 
    vec3 indirect = traceCone(volumeTexture, voxelPosition, dir, vctSteps, vctConeAngle, vctConeInitialHeight);
    color = vec4(indirect, sphereContrib);
}
