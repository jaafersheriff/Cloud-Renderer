#version 330 core

in vec4 worldPos;

uniform bool isOutline;
uniform float alpha;

/* Voxel data */
uniform vec4 voxelData;

out vec4 color;

void main() {
    if (isOutline) {
        color = vec4(1, 0, 0, 1);
        return;
    }

    color = voxelData;
    color.a = alpha;
}