#include "CloudVolume.hpp"

#include "Shaders/GLSL.hpp"

CloudVolume::CloudVolume(int dim, glm::vec2 bounds, glm::vec3 position, int mips) {
    for (int i = 0; i < dim*dim*dim; i++) {
        this->voxelData.push_back({
            Spatial(),
            glm::vec4()
            });
    }
    this->dimension = dim;
    this->position = position;
    this->xBounds = bounds;
    this->yBounds = bounds;
    this->zBounds = bounds;
    this->voxelSize = glm::vec3(
        (xBounds.y - xBounds.x) / dimension,
        (yBounds.y - yBounds.x) / dimension,
        (zBounds.y - zBounds.x) / dimension);
    this->levels = mips;

    /* Init volume */
    CHECK_GL_CALL(glGenTextures(1, &volId));
    CHECK_GL_CALL(glBindTexture(GL_TEXTURE_3D, volId));
    CHECK_GL_CALL(glTexStorage3D(GL_TEXTURE_3D, mips, GL_RGBA16F, dimension, dimension, dimension)); // immutable

    CHECK_GL_CALL(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
    CHECK_GL_CALL(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

    CHECK_GL_CALL(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    CHECK_GL_CALL(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    CHECK_GL_CALL(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE));

    clearGPU();
    CHECK_GL_CALL(glBindTexture(GL_TEXTURE_3D, 0));
}

/* Add a billboard */
void CloudVolume::addCloudBoard(Spatial &s) {
    this->cloudBoards.push_back(s);
}

/* Sort billboards by distance to a point */
void CloudVolume::sortBoards(glm::vec3 orig) {
    for (unsigned int i = 0; i < cloudBoards.size(); i++) {
        int minIdx = i;
        for (unsigned int j = i + 1; j < cloudBoards.size(); j++) {
            if (glm::distance(cloudBoards[minIdx].position, orig) < glm::distance(cloudBoards[j].position, orig)) {
                minIdx = j;
            }
        }
        if (i != minIdx) {
            Spatial tmp = cloudBoards[i];
            cloudBoards[i] = cloudBoards[minIdx];
            cloudBoards[minIdx] = tmp;
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
            voxelData[i].spatial.position = this->position + reverseVoxelIndex(voxelIndex, range);
            voxelData[i].spatial.scale = voxelSize;
            voxelData[i].data.r = r;
            voxelData[i].data.g = g;
            voxelData[i].data.b = b;
            voxelData[i].data.a = a;
        }
        /* Otherwise reset data */
        else {
            voxelData[i].spatial.position.x = 0.f;
            voxelData[i].spatial.position.y = 0.f;
            voxelData[i].spatial.position.z = 0.f;
            voxelData[i].spatial.scale.x = 0.f;
            voxelData[i].spatial.scale.y = 0.f;
            voxelData[i].spatial.scale.z = 0.f;
            voxelData[i].data.x = 0.f;
            voxelData[i].data.y = 0.f;
            voxelData[i].data.z = 0.f;
            voxelData[i].data.w = 0.f;
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
