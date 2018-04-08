#pragma once
#ifndef _VOLUME_SHADER_HPP_
#define _VOLUME_SHADER_HPP_

#include "Shader.hpp"

#include "Model/Mesh.hpp"
#include "Spatial.hpp"

class VolumeShader : public Shader {
    public:
        struct Voxel {
            /* Position and scale in world-space */
            Spatial spatial;
            /* Data stored in voxel */
            glm::vec4 data;
        };

        VolumeShader(std::string vertex, std::string fragment) :
            Shader(vertex, fragment)
        {}

        bool init(int, glm::vec2, glm::vec2, glm::vec2, Spatial *);

        /* Generate 3D volume */
        void clearVolume();
        void voxelize(glm::mat4, glm::mat4, glm::vec3, GLuint);
        void renderMesh(glm::mat4, glm::mat4, glm::vec3, bool, GLuint);

        std::vector<Voxel> & getVoxelData() { return voxelData; }
        void updateVoxelData();

        glm::vec2 xBounds;
        glm::vec2 yBounds;
        glm::vec2 zBounds;
        int volumeSize;
        float radius;

        float normalStep = 0.2f;
        float visibilityContrib = 0.02f;

     private:
        void initVolume();
        glm::ivec3 get3DIndices(int);
        glm::vec3 reverseVoxelIndex(glm::ivec3);

        /* Volume vars */
        GLuint volumeHandle;
        Spatial * volQuad;
        std::vector<Voxel> voxelData;

};

#endif