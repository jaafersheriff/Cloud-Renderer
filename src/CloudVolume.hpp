#pragma once
#ifndef _CLOUD_VOLUME_HPP_
#define _CLOUD_VOLUME_HPP_

#include <glad/glad.h>

#include "glm/glm.hpp"

#include <vector>

class CloudVolume {
    public:
        CloudVolume(int, glm::vec2, glm::vec3, int);

        void addCloudBoard(glm::vec3, float);
        void sortBoards(glm::vec3);
        void updateVoxelData();
        void clearGPU();

        glm::vec3 position;     // cloud object position

        glm::vec2 xBounds;      // Min and max x-mapping in world-space
        glm::vec2 yBounds;      // Min and max y-mapping in world-space
        glm::vec2 zBounds;      // Min and max z-mapping in world-space
        int dimension;          // Voxels per dimension 
        glm::vec3 voxelSize;    // World-size of individual voxels
        int levels;             // Mipmap levels

        /* Billboard */
        std::vector<glm::vec3> billboardPositions;
        std::vector<float> billboardScales;

        /* Voxels */
        std::vector<glm::vec3> voxelPositions;
        std::vector<glm::vec4> voxelData;

        GLuint volId;
        int voxelCount = 0;
    private:
        glm::ivec3 get3DIndices(int);
        glm::vec3 reverseVoxelIndex(const glm::ivec3 &, const glm::vec3 &);
};

#endif
