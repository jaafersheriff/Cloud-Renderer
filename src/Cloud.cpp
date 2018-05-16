#include "Cloud.hpp"

#include "Util.hpp"

Cloud::Cloud(int billboards, glm::vec3 position, glm::vec3 scale, float offset, float bounds, int volDim, int volMips) {
    for (int i = 0; i < billboards; i++) {
        Volume *v = new Volume(volDim, glm::vec2(-bounds, bounds), Util::genRandomVec3(-offset, offset), scale, volMips);
        this->volumes.push_back(v);
        this->voxelData.push_back(&v->voxelData);
    }
    this->spatial.position = position;
    this->spatial.scale = scale;
    this->xBounds = bounds;
    this->yBounds = bounds;
    this->zBounds = bounds;
}

void Cloud::clearCPU() {
    for (auto vol : volumes) {
        vol->clearCPU();
    }
}

void Cloud::clearGPU() {
    for (auto vol : volumes) {
        vol->clearGPU();
    }
}

void Cloud::updateVoxelData() {
    for (auto vol : volumes) {
        vol->updateVoxelData(this->spatial.position);
    }
}

void Cloud::updateBounds() {
    for (auto vol : volumes) {
        vol->xBounds = glm::vec2(-xBounds, xBounds);
        vol->yBounds = glm::vec2(-yBounds, yBounds);
        vol->zBounds = glm::vec2(-zBounds, zBounds);
    }
}

