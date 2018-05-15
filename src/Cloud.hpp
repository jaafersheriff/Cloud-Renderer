#pragma once
#ifndef _CLOUD_HPP_
#define _CLOUD_HPP_

#include "Volume.hpp"

class Cloud {
    public:
        Cloud(int, glm::vec3, glm::vec3, float, int, glm::vec2, glm::vec2, int);
        std::vector<Volume *> volumes;
        glm::vec3 position;
        glm::vec3 scale;

        void clearCPU();
        void clearGPU();
        void updateVoxelData();
        std::vector<std::vector<Volume::Voxel> *> voxelData;
};

#endif