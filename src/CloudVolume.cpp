#include "CloudVolume.hpp"

#include "Library.hpp"
#include "Util.hpp"
#include "Shaders/GLSL.hpp"

CloudVolume::CloudVolume(int dim, glm::vec2 bounds, glm::vec3 position, int mips) {
    this->dimension = dim;
    this->position = position;
    this->xBounds = bounds;
    this->yBounds = bounds;
    this->zBounds = bounds;
    this->levels = mips;

    /* Init volume */
    CHECK_GL_CALL(glGenTextures(1, &volId));
    CHECK_GL_CALL(glBindTexture(GL_TEXTURE_3D, volId));
    CHECK_GL_CALL(glTexStorage3D(GL_TEXTURE_3D, mips, GL_R8, dimension, dimension, dimension)); // immutable
    CHECK_GL_CALL(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
    CHECK_GL_CALL(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    CHECK_GL_CALL(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    CHECK_GL_CALL(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    CHECK_GL_CALL(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE));
    clearGPU();
    CHECK_GL_CALL(glBindTexture(GL_TEXTURE_3D, 0));

    /* Init instanced quad */
    int numVoxels = dim * dim * dim;
    this->instancedQuad = Library::createQuad();
    CHECK_GL_CALL(glBindVertexArray(instancedQuad->vaoId));
    CHECK_GL_CALL(glGenBuffers(1, &instancedQuadPosVBO));
    CHECK_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, instancedQuadPosVBO));
    CHECK_GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * numVoxels, nullptr, GL_DYNAMIC_DRAW));
    CHECK_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0)); 
    CHECK_GL_CALL(glEnableVertexAttribArray(2));
    CHECK_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, instancedQuadPosVBO));
    CHECK_GL_CALL(glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0));
    CHECK_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));	
    CHECK_GL_CALL(glVertexAttribDivisor(2, 1)); 
    CHECK_GL_CALL(glGenBuffers(1, &instancedQuadScaleVBO));
    CHECK_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, instancedQuadScaleVBO));
    CHECK_GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * numVoxels, nullptr, GL_DYNAMIC_DRAW));
    CHECK_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0)); 
    CHECK_GL_CALL(glEnableVertexAttribArray(3));
    CHECK_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, instancedQuadScaleVBO));
    CHECK_GL_CALL(glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void*)0));
    CHECK_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));	
    CHECK_GL_CALL(glVertexAttribDivisor(3, 1));

    range = glm::vec3(
        xBounds.y - xBounds.x,
        yBounds.y - yBounds.x,
        zBounds.y - zBounds.x);
    voxelSize = range / (float)dimension;
}

/* Add a billboard */
void CloudVolume::addCloudBoard(glm::vec3 &pos, float &scale) {
    billboards.count++;
    billboards.positions.push_back(pos);
    billboards.scales.push_back(scale);
}

/* Sort billboards by distance to a point */
void CloudVolume::sortBoards(glm::vec3 point) {
    for (unsigned int i = 0; i < billboards.count; i++) {
        int minIdx = i;
        for (unsigned int j = i + 1; j < billboards.count; j++) {
            if (glm::distance(this->position + billboards.positions[minIdx], point) < glm::distance(this->position + billboards.positions[j], point)) {
                minIdx = j;
            }
        }
        if (i != minIdx) {
            glm::vec3 tmpPos = billboards.positions[i];
            billboards.positions[i] = billboards.positions[minIdx];
            billboards.positions[minIdx] = tmpPos;
            float tmpScale = billboards.scales[i];
            billboards.scales[i] = billboards.scales[minIdx];
            billboards.scales[minIdx] = tmpScale;
        }
    }
}

void CloudVolume::update() {
    /* Reupload billboard positions and scales */
    uploadBillboards();

    range = glm::vec3(
        xBounds.y - xBounds.x,
        yBounds.y - yBounds.x,
        zBounds.y - zBounds.x);
    voxelSize = range / (float)dimension;
}

/* Reset GPU volume */
void CloudVolume::clearGPU() {
    for (int i = 0; i < levels; i++) {
        CHECK_GL_CALL(glClearTexImage(volId, i, GL_RED, GL_FLOAT, nullptr));
    }
}

// Assume 4 bytes per voxel
glm::ivec3 CloudVolume::get3DIndices(const int index) const {
	int line = dimension;
	int slice = dimension  * line;
	int z = index / slice;
	int y = (index - z * slice) / line;
	int x = (index - z * slice - y * line);
	return glm::ivec3(x, y, z);
}

glm::vec3 CloudVolume::reverseVoxelIndex(const glm::ivec3 &voxelIndex) const {
    float x = float(voxelIndex.x) * range.x / dimension + xBounds.x;
    float y = float(voxelIndex.y) * range.y / dimension + yBounds.x;
    float z = float(voxelIndex.z) * range.z / dimension + zBounds.x;

    return glm::vec3(x, y, z);
}

void CloudVolume::regenerateBillboards(int count, glm::vec3 minOffset, glm::vec3 maxOffset, float minScale, float maxScale) {
    billboards.minOffset = minOffset;
    billboards.maxOffset = maxOffset;
    billboards.minScale = minScale;
    billboards.maxScale = maxScale;
    billboards.positions.clear();
    billboards.scales.clear();
    billboards.count = 0;
    for (int i = 0; i < count; i++) {
        glm::vec3 position = Util::genRandomVec3(minOffset.x, maxOffset.x, minOffset.y, maxOffset.y, minOffset.z, maxOffset.z);
        float scale = Util::genRandom(minScale, maxScale);
        addCloudBoard(position, scale);
    }
}

void CloudVolume::resetBillboards() {
    regenerateBillboards(billboards.count, billboards.minOffset, billboards.maxOffset, billboards.minScale, billboards.maxScale);
}

void CloudVolume::uploadBillboards() {
    if (!billboards.count) {
        return;
    }
    
    CHECK_GL_CALL(glBindVertexArray(instancedQuad->vaoId));

    /* Reupload billboard positions */
    CHECK_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, instancedQuadPosVBO));
    CHECK_GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * billboards.positions.size(), &billboards.positions[0], GL_DYNAMIC_DRAW));

    /* Reupload billboard scales */
    CHECK_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, instancedQuadScaleVBO));
    std::vector<float> scales;
    if (fluffiness != 1.f) {
        for (float scale : billboards.scales) {
            scales.push_back(scale * fluffiness);
        }
    }
    else {
        scales = billboards.scales;
    }
    CHECK_GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * scales.size(), &scales[0], GL_DYNAMIC_DRAW));

    CHECK_GL_CALL(glBindVertexArray(0));
}
