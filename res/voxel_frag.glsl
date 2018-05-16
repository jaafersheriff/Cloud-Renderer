#version 330 core

uniform bool isOutline;
uniform float alpha;

/* Voxel data */
uniform vec4 voxelData;

out vec4 color;

void main() {
    if (isOutline) {
        color = vec4(0.5, 0.5, 0.5, 1);
        return;
    }

    color = voxelData;
    color.a = alpha;
}