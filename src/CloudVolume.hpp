#pragma once
#ifndef _CLOUD_VOLUME_HPP_
#define _CLOUD_VOLUME_HPP_

#include <glad/glad.h>

#include "glm/glm.hpp"

#include <vector>

class Mesh;
class CloudVolume {
    public:
        struct Billboards {
            std::vector<glm::vec3> positions;
            std::vector<float> scales;

            int count = 0;
            glm::vec3 minOffset = glm::vec3(-1.f);
            glm::vec3 maxOffset = glm::vec3(1.f);
            float minScale = 1.f;
            float maxScale = 1.f;
        };

        CloudVolume(int, glm::vec2, glm::vec3, int);

        void update();
        void clearGPU();

        void addCloudBoard(glm::vec3 &, float &);
        void sortBoards(glm::vec3);

        glm::vec3 position;     // cloud object position

        glm::vec2 xBounds;      // Min and max x-mapping in world-space
        glm::vec2 yBounds;      // Min and max y-mapping in world-space
        glm::vec2 zBounds;      // Min and max z-mapping in world-space
        int dimension;          // Voxels per dimension 
        glm::vec3 range;
        glm::vec3 voxelSize;    // World-size of individual voxels
        int levels;             // Mipmap levels

        Mesh * instancedQuad;
        GLuint instancedQuadPosVBO;
        GLuint instancedQuadScaleVBO;
        Billboards billboards;
        void uploadBillboards();
        void regenerateBillboards(int, glm::vec3, glm::vec3, float, float);
        void resetBillboards();
        float fluffiness = 1.f;

        GLuint volId;
        glm::ivec3 get3DIndices(int) const;
        glm::vec3 reverseVoxelIndex(const glm::ivec3 &) const;
};

#endif
