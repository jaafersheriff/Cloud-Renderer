#include "CloudVolume.hpp"

#include "Library.hpp"
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
void CloudVolume::addCloudBoard(glm::vec3 pos, float scale) {
    this->billboardPositions.push_back(pos);
    this->billboardScales.push_back(scale);
}

/* Sort billboards by distance to a point */
void CloudVolume::sortBoards(glm::vec3 point) {
    for (unsigned int i = 0; i < billboardPositions.size(); i++) {
        int minIdx = i;
        for (unsigned int j = i + 1; j < billboardPositions.size(); j++) {
            if (glm::distance(this->position + billboardPositions[minIdx], point) < glm::distance(this->position + billboardPositions[j], point)) {
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

void CloudVolume::uploadBillboards() {
    if (!billboardPositions.size()) {
        return;
    }
    
    CHECK_GL_CALL(glBindVertexArray(instancedQuad->vaoId));

    /* Reupload billboard positions */
    CHECK_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, instancedQuadPosVBO));
    CHECK_GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * billboardPositions.size(), &billboardPositions[0], GL_DYNAMIC_DRAW));

    /* Reupload billboard scales */
    CHECK_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, instancedQuadScaleVBO));
    CHECK_GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * billboardScales.size(), &billboardScales[0], GL_DYNAMIC_DRAW));

    CHECK_GL_CALL(glBindVertexArray(0));
}
