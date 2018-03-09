#version 440 core

in vec3 fragPos;
in vec2 fragTex;

uniform vec2 xBounds;
uniform vec2 yBounds;
uniform vec2 zBounds;
uniform int voxelSize;
uniform bool voxelize;

layout(binding=1, rgba32f) uniform image3D volume;

out vec4 color;

/* Linear map from aribtray box(?) in world space to 3D volume 
 * Voxel indices: [0, voxelSize - 1] */
ivec3 voxelIndex(vec3 pos) {
    float rangeX = xBounds.y - xBounds.x;
    float rangeY = yBounds.y - yBounds.x;
    float rangeZ = zBounds.y - zBounds.x;

	float x = voxelSize * ((pos.x - xBounds.x) / rangeX);
	float y = voxelSize * ((pos.y - yBounds.x) / rangeY);
	float z = voxelSize * ((pos.z - zBounds.x) / rangeZ);

	return ivec3(x, y, z);
}

void main() {
    color = vec4(1, 0, 0, 1);
    if(voxelize) {
        ivec3 i = voxelIndex(fragPos);
        imageStore(volume, i, vec4(1, 1, 1, 1));

		/*for(int xx=0;xx<3;xx++)
			for(int yy=0;yy<3;yy++)
				for(int zz=0;zz<3;zz++)
					imageStore(volume, ivec3(3+xx*2,3+yy*2,3+zz*2), vec4(1,0,0,0));*/
    }
}
