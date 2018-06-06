#pragma once
#ifndef _CLOUD_VOLUME_HPP_
#define _CLOUD_VOLUME_HPP_

#include <glad/glad.h>

#include "Spatial.hpp"

#include <vector>

class CloudVolume {
    public:
        struct Voxel {
            Spatial spatial;    // Position and scale in world-space
            glm::vec4 data;     // Data stored in voxel
        };

        CloudVolume(int, glm::vec2, glm::vec3, int);

        void addCloudBoard(Spatial &);
        void sortBoards(glm::vec3);
        void updateVoxelData();
        void clearGPU();

        glm::vec3 position;                 // cloud object position
        std::vector<Spatial> cloudBoards;   // billboard spatials in relation to cloud spatial

        glm::vec2 xBounds;      // Min and max x-mapping in world-space
        glm::vec2 yBounds;      // Min and max y-mapping in world-space
        glm::vec2 zBounds;      // Min and max z-mapping in world-space
        int dimension;          // Voxels per dimension 
        glm::vec3 voxelSize;    // World-size of individual voxels
        int levels;             // Mipmap levels

        std::vector<Voxel> voxelData;

        GLuint volId;
        int voxelCount = 0;
    private:
        glm::ivec3 get3DIndices(int);
        glm::vec3 reverseVoxelIndex(const glm::ivec3 &, const glm::vec3 &);
};

#endif
