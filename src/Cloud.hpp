#pragma once
#ifndef _CLOUD_HPP_
#define _CLOUD_HPP_

#include "Volume.hpp"

class Cloud {
    public:
        Cloud(int, glm::vec3, glm::vec3, float, float, int, int);

        std::vector<Volume *> volumes;
        Spatial spatial;
        float xBounds = 0.1f;
        float yBounds = 0.1f;
        float zBounds = 0.1f;

        void clearCPU();
        void clearGPU();
        void updateVoxelData();
        std::vector<std::vector<Volume::Voxel> *> voxelData;
};

#endif