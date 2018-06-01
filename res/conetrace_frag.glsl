#version 440 core

#define PI 3.14159265359f

in vec3 fragPos;
in vec3 fragNor;
in vec2 fragTex;

uniform mat4 V;
uniform vec3 lightPos;

uniform vec3 center;
uniform float scale;

uniform int voxelDim;
uniform vec2 xBounds;
uniform vec2 yBounds;
uniform vec2 zBounds;
uniform sampler3D volumeTexture;

uniform int vctSteps;
uniform float vctConeAngle;
uniform float vctConeInitialHeight;
uniform float vctLodOffset;
uniform float vctDownScaling;

uniform sampler3D noisemap;
uniform mat4 g_OctaveOffsets;

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

vec4 traceCone(sampler3D voxelTexture, vec3 position, vec3 direction, int steps, float coneAngle, float coneHeight) {
    direction = normalize(direction);
    direction /= voxelDim;
    position /= voxelDim;
    vec4 color = vec4(0);
    for (int i = 1; i <= steps; i++) {
        float coneRadius = coneHeight * tan(coneAngle / 2.f);
        float lod = log2(max(1.f, 2.f * coneRadius));
        vec4 sampleColor = textureLod(voxelTexture, position + coneHeight * direction, lod + vctLodOffset);
        color += sampleColor * float(i)/(steps*vctDownScaling);
        // TODO : linear scaling
        coneHeight += coneRadius;
    }


    return color;
}


float saturate(float x) {
    return clamp(x, 0, 1);
}



#define MAX_STEPS 8
#define DIST_BIAS 0.01
bool RaySphereIntersect(vec3 rO, vec3 rD, vec3 sO, float sR, inout float tnear, inout float tfar)
{
    vec3 delta = rO - sO;
    float A = dot(rD, rD);
    float B = 2 * dot(delta, rD);
    float C = dot(delta, delta) - sR*sR;
    float disc = B*B - 4.0*A*C;
    if (disc < DIST_BIAS)
	{
        return false;
    }

	else
	{
        float sqrtDisc = sqrt(disc);
        tnear = (-B - sqrtDisc) / (2 * A);
        tfar = (-B + sqrtDisc) / (2 * A);
        return true;
    }

}
vec4 Noise3D(vec3 uv, int octaves)
{
    vec4 noiseVal = vec4(0, 0, 0, 0);
    vec4 octaveVal = vec4(0, 0, 0, 0);
    vec3 uvOffset;
    float freq = 1;
    float pers = 1;
    for (int i = 0; i<octaves; i++)
	{
        uvOffset = uv + g_OctaveOffsets[i].xyz;
        octaveVal = texture(noisemap, uvOffset*freq, 0);
        //test
		noiseVal += pers * octaveVal;
        freq *= 3.0;
        pers *= 0.5;
    }


	noiseVal.a = abs(noiseVal.a);
    //turbulence

	return noiseVal;
}


void main() {
    float radius = scale/2;
    /* Spherical distance - 1 at center of billboard, 0 at edges */
    float sphereContrib = (distance(center, fragPos)/radius);
    sphereContrib = sqrt(max(0, 1 - sphereContrib * sphereContrib));
    // if (sphereContrib <= 0.f) {
    //     discard;
    // }


    /* Start at voxel closest to camera */
    vec3 billboardNormal = normalize(fragNor);
    vec3 pos = fragPos + billboardNormal * radius * sphereContrib;
    // soft particles
    vec3 worldPosition = fragPos;
    vec3 viewRay = normalize(vec3(V._13, V._23, V._33));
    vec4 sphereO = vec4(center,1);
    float rad = radius;
    float tnear, tfar;
    if (!RaySphereIntersect(worldPosition, viewRay, sphereO.xyz, rad, tnear, tfar))
    	discard;
    vec3 worldNear = worldPosition + viewRay*tnear;
    vec3 worldFar = worldPosition + viewRay*tfar;
    vec4 viewNear = vec4(worldNear, 1)* V;
    vec4 viewFar = vec4(worldFar, 1)* V;
    float currentDepth = viewNear.z/ viewNear.w;
    float farDepth = viewFar.z / viewFar.w;
    vec3 unitTex = (worldNear - sphereO.xyz) / rad;
    float g_noiseSize=40.0;
    float fNoiseSizeAdjust = 1 / g_noiseSize;
    vec3 localTexNear = worldNear * fNoiseSizeAdjust;
    vec3 localTexFar = worldFar * fNoiseSizeAdjust;
    float g_stepSize = 0.01;
    float g_noiseOpacity = 20.0;
    vec4 g_directional1 = vec4(0.992, 1.0, 0.880, 0.0);
    float iSteps = length(localTexFar - localTexNear) / g_stepSize;
    iSteps = min(iSteps, MAX_STEPS - 2) + 2;
    vec3 currentTex = localTexNear;
    vec3 localTexDelta = (localTexFar - localTexNear) / (iSteps - 1);
    float depthDelta = (farDepth - currentDepth) / (iSteps - 1);
    float opacityAdjust = g_noiseOpacity / (iSteps - 1);
    float lightAdjust = 1.0 / (iSteps - 1);
    float lifePower = 0;
    float runningOpacity = 0;
    vec4 runningLight = vec4(0, 0, 0, 0);
    float depthFade = 1.0;
    for (int i = 0; i<iSteps; i++)
{
        vec4 noiseCell = Noise3D(currentTex, 4);
        noiseCell.xyz += normalize(unitTex);
        noiseCell.xyz = normalize(noiseCell.xyz);
        float lenSq = dot(unitTex, unitTex);
        float falloff = 1.0 - lenSq;
        //1 - len^2 falloff
	float localOpacity = noiseCell.a*falloff;
        runningOpacity += localOpacity;
        vec4 localLight = g_directional1*saturate(dot(noiseCell.xyz, vec3(0, 1, 0))*0.5 + 0.5);
        localLight = vec4(saturate(dot(noiseCell.xyz, vec3(0, 1, 0))*0.5 + 0.5));
        runningLight += localLight;
        currentTex += localTexDelta;
        unitTex += localTexDelta;
        currentDepth += depthDelta;
    }

    float alpha = 1 - pow(length(fragTex - vec2(0.5,0.5))*2.0,1);
    vec3 partcolor = vec3(lifePower,lifePower, lifePower)+vec3(0.5,0.5,0.5);
    partcolor.r = saturate(partcolor.r);
    partcolor.g = saturate(partcolor.g);
    partcolor.b = saturate(partcolor.b);
    vec4 col = (vec4(partcolor, 1)* runningLight*lightAdjust)*0.9 + 0.2;
    runningOpacity = saturate(runningOpacity*opacityAdjust);
    color = vec4(col.rgb, runningOpacity*alpha);

    /* Cone trace */
    vec3 voxelPosition = calculateVoxelLerp(pos);
    vec3 dir = lightPos - pos;
    vec4 indirect = traceCone(volumeTexture, voxelPosition, dir, vctSteps, vctConeAngle, vctConeInitialHeight);
    /* Output */
    color.rgb *= indirect.xyz; 
}

