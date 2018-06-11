#include "CloudVolume.hpp"

#include "Shaders/GLSL.hpp"

CloudVolume::CloudVolume(int dim, glm::vec2 bounds, glm::vec3 position, int mips) {
    this->dimension = dim;
    this->position = position;
    this->xBounds = bounds;
    this->yBounds = bounds;
    this->zBounds = bounds;
    this->levels = mips;
    this->voxelSize = glm::vec3(
        (xBounds.y - xBounds.x) / dimension,
        (yBounds.y - yBounds.x) / dimension,
        (zBounds.y - zBounds.x) / dimension);

    int numVoxels = dim * dim * dim;
    this->voxelPositions.resize(numVoxels);
    this->voxelData.resize(numVoxels);

    /* Init volume */
    CHECK_GL_CALL(glGenTextures(1, &volId));
    CHECK_GL_CALL(glBindTexture(GL_TEXTURE_3D, volId));
    CHECK_GL_CALL(glTexStorage3D(GL_TEXTURE_3D, mips, GL_RGBA8, dimension, dimension, dimension)); // immutable

    CHECK_GL_CALL(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
    CHECK_GL_CALL(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

    CHECK_GL_CALL(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    CHECK_GL_CALL(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    CHECK_GL_CALL(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE));

    clearGPU();
    CHECK_GL_CALL(glBindTexture(GL_TEXTURE_3D, 0));
}

/* Add a billboard */
void CloudVolume::addCloudBoard(glm::vec3 pos, float scale) {
    this->billboardPositions.push_back(pos);
    this->billboardScales.push_back(scale);
}

/* Sort billboards by distance to a point */
void CloudVolume::sortBoards(glm::vec3 point) {
    for (unsigned int i = 0; i < billboardPositions.size(); i++) {
        int minIdx = i;
        for (unsigned int j = i + 1; j < billboardPositions.size(); j++) {
            if (glm::distance(billboardPositions[minIdx], point) < glm::distance(billboardPositions[j], point)) {
                minIdx = j;
            }
        }
        if (i != minIdx) {
            glm::vec3 tmpPos = billboardPositions[i];
            billboardPositions[i] = billboardPositions[minIdx];
            billboardPositions[minIdx] = tmpPos;
            float tmpScale = billboardScales[i];
            billboardScales[i] = billboardScales[minIdx];
            billboardScales[minIdx] = tmpScale;
        }
    }
}

/* Reset GPU volume */
void CloudVolume::clearGPU() {
    for (int i = 0; i < levels; i++) {
        CHECK_GL_CALL(glClearTexImage(volId, i, GL_RGBA, GL_FLOAT, nullptr));
    }
}

void CloudVolume::updateVoxelData() {
    /* Pull volume data out of GPU */
    std::vector<float> buffer(voxelData.size() * 4);
    CHECK_GL_CALL(glBindTexture(GL_TEXTURE_3D, volId));
    CHECK_GL_CALL(glGetTexImage(GL_TEXTURE_3D, 0, GL_RGBA, GL_FLOAT, buffer.data()));
    CHECK_GL_CALL(glBindTexture(GL_TEXTURE_3D, 0));

    /* Size of voxels in world-space */
    glm::vec3 range = glm::vec3(
        xBounds.y - xBounds.x,
        yBounds.y - yBounds.x,
        zBounds.y - zBounds.x);
    voxelSize = range / (float)dimension;
    voxelCount = 0;
    for (unsigned int i = 0; i < voxelData.size(); i++) {
        /* Update voxel data if data exists */
        float r = buffer[4*i + 0];
        float g = buffer[4*i + 1];
        float b = buffer[4*i + 2];
        float a = buffer[4*i + 3];
        if (r || g || b || a) {
            voxelCount++;
            glm::ivec3 voxelIndex = get3DIndices(4*i);
            voxelPositions[i] = this->position + reverseVoxelIndex(voxelIndex, range);
            voxelData[i].r = r;
            voxelData[i].g = g;
            voxelData[i].b = b;
            voxelData[i].a = a;
        }
        /* Otherwise reset data */
        else {
            voxelPositions[i].x = 0.f;
            voxelPositions[i].y = 1000000.f; // ensures instanced voxels won't be rendered
            voxelPositions[i].z = 0.f;
            voxelData[i].x = 0.f;
            voxelData[i].y = 0.f;
            voxelData[i].z = 0.f;
            voxelData[i].w = 0.f;
        }
    }
}

// Assume 4 bytes per voxel
glm::ivec3 CloudVolume::get3DIndices(const int index) {
	int line = dimension * 4;
	int slice = dimension  * line;
	int z = index / slice;
	int y = (index - z * slice) / line;
	int x = (index - z * slice - y * line) / 4;
	return glm::ivec3(x, y, z);
}

glm::vec3 CloudVolume::reverseVoxelIndex(const glm::ivec3 &voxelIndex, const glm::vec3 &range) {
    float x = float(voxelIndex.x) * range.x / dimension + xBounds.x;
    float y = float(voxelIndex.y) * range.y / dimension + yBounds.x;
    float z = float(voxelIndex.z) * range.z / dimension + zBounds.x;

    return glm::vec3(x, y, z);
}
