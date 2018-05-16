#pragma once
#ifndef _CLOUD_HPP_
#define _CLOUD_HPP_

#include "Volume.hpp"

class Cloud {
    public:
        Cloud(int, glm::vec3, glm::vec3, float, float, int, int);

        std::vector<Volume *> volumes;
        Spatial spatial;
        float xBounds;
        float yBounds;
        float zBounds;

        void clearCPU();
        void clearGPU();
        void updateVoxelData();
        void updateBounds();
        std::vector<std::vector<Volume::Voxel> *> voxelData;
};

#endif