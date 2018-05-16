#pragma once
#ifndef _VOLUME_HPP_
#define _VOLUME_HPP_

#include <glad/glad.h>

#include "Spatial.hpp"

#include <vector>

class Volume {
    public:
        struct Voxel {
            Spatial spatial;    // Position and scale in world-space
            glm::vec4 data;     // Data stored in voxel
        };

        Volume(int, glm::vec2, glm::vec3, int);

        void addCloudBoard(Spatial);
        void updateVoxelData();
        void clearGPU();
        void clearCPU();

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
        glm::vec3 reverseVoxelIndex(glm::ivec3);
};

#endif
