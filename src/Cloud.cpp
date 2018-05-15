#include "Cloud.hpp"

#include "Util.hpp"

Cloud::Cloud(int billboards, glm::vec3 position, glm::vec3 scale, float offset, int volDim, glm::vec2 volBounds, glm::vec2 volScale, int volMips) {
    for (int i = 0; i < billboards; i++) {
        Volume *v = new Volume(volDim, volBounds, Util::genRandomVec3(-offset, offset), volScale, volMips);
        this->volumes.push_back(v);
        this->voxelData.push_back(&v->voxelData);
    }
    this->spatial.position = position;
    this->spatial.scale = scale;
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
        vol->updateVoxelData();
    }
}

