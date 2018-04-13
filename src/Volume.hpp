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

        Volume(int, glm::vec2, glm::vec3, glm::vec2, int);

        std::vector<Voxel> & getVoxelData() { return voxelData; }
        void updateVoxelData();
        void clear();

        glm::vec3 quadPosition; // world-position of representing billboard
        glm::vec2 quadScale;    // size of representing billboard
        glm::vec2 xBounds;  // Min and max x-mapping in world-space
        glm::vec2 yBounds;  // Min and max y-mapping in world-space
        glm::vec2 zBounds;  // Min and max z-mapping in world-space
        int dimension;      // Voxels per dimension 

        int voxelCount = 0; // Count of voxels containing any data
        glm::vec3 voxelSize;    // World-size of individual voxels
        int levels;         // Mipmap levels

        GLuint volId;

    private:
        std::vector<Voxel> voxelData;

        glm::ivec3 get3DIndices(int);
        glm::vec3 reverseVoxelIndex(glm::ivec3);
};

#endif
