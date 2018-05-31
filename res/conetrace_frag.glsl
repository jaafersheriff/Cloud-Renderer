#version 440 core

#define PI 3.14159265359f

in vec3 fragPos;
in vec3 fragNor;
in vec2 fragTex;

uniform vec3 center;
uniform float scale;

uniform int voxelDim;
uniform vec2 xBounds;
uniform vec2 yBounds;
uniform vec2 zBounds;
uniform vec3 lightPos;

uniform sampler2D tex;
uniform sampler3D volumeTexture;
uniform int vctSteps;
uniform float vctConeAngle;
uniform float vctConeInitialHeight;
uniform float vctLodOffset;
uniform float vctDownScaling;

uniform int numBoards;

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
        color += sampleColor * float(i)/(steps*vctDownScaling); // TODO : linear scaling
        coneHeight += coneRadius;
    }

    return color;
}

float atan2(float x, float y) {
    bool s = (abs(x) > abs(y));
    return mix(PI/2.f - atan(x,y), atan(y,x), s);
}

vec2 getSphereUV(vec3 surfacePos, vec3 sphereCenter) {
    vec3 normal = normalize(surfacePos - sphereCenter);
    float u = 0.5f + atan2(normal.z, normal.x) / (2.f * PI);
    float v = 0.5f + asin(normal.y) / PI;
    return vec2(u, v);
}

vec4 Noise3D( vec3 uv, int octaves ) {
    vec4 noiseVal = vec4(0,0,0,0);
    vec4 octaveVal = vec4(0,0,0,0);
    vec3 uvOffset;
    float freq = 1;
    float pers = 1;

    for( int i=0; i<octaves; i++ ) {
        // uvOffset = uv + g_OctaveOffsets[i].xyz;
        // octaveVal = g_txVolumeDiff.SampleLevel( g_samVolume, uvOffset*freq, 0 );
        noiseVal += pers * octaveVal;
        
        freq *= 3.0;
        pers *= 0.5;
    }
    
    noiseVal.a = abs( noiseVal.a ); //turbulence
    
    return noiseVal;
}
 
void main() {
    float radius = scale/2;

    /* Spherical distance - 1 at center of billboard, 0 at edges */
    float sphereContrib = (distance(center, fragPos)/radius);
    sphereContrib = sqrt(max(0, 1 - sphereContrib * sphereContrib));
    if (sphereContrib <= 0.f) {
        discard;
    }

    /* Start at voxel closest to camera */
    vec3 billboardNormal = normalize(fragNor);
    vec3 pos = fragPos + billboardNormal * radius * sphereContrib;

    vec2 uv = getSphereUV(pos, center);
    vec4 scolor = texture(tex, uv);

    // vec3 currentTex = pos * (1/40);
    // vec4 noiseCell = Noise3D(currentTex, 4);
    // noiseCell.xyz += normalize((pos - center) / radius);
    // noiseCell.xyz = normalize(noiseCell.xyz);
    // color = vec4(noiseCell,1);
    // return;

    vec3 voxelPosition = calculateVoxelLerp(pos);
    vec3 dir = lightPos - pos; 
    vec4 indirect = traceCone(volumeTexture, voxelPosition, dir, vctSteps, vctConeAngle, vctConeInitialHeight);
    color = indirect * scolor; //vec4(indirect.x * scolor.xyz, scolor.x * indirect.x);
}
