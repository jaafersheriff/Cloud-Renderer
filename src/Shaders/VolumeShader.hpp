#pragma once
#ifndef _VOLUME_SHADER_HPP_
#define _VOLUME_SHADER_HPP_

#include "Shader.hpp"

#include "Model/Mesh.hpp"
#include "Spatial.hpp"

class VolumeShader : public Shader {
    public:
        VolumeShader(std::string vertex, std::string fragment) :
            Shader(vertex, fragment)
        {}

        bool init(int, glm::vec2, glm::vec2, glm::vec2, Spatial *);

        /* Generate 3D volume */
        void voxelize(Mesh *);
        void clearVolume();
        void renderMesh(Mesh *, bool);

        /* Getters */
        std::vector<glm::vec3> & getVoxelData() { return voxelData; }

        bool activeVoxelize = false;
        glm::vec2 xBounds;
        glm::vec2 yBounds;
        glm::vec2 zBounds;
        int volumeSize;
 
     private:
        void initVolume();
        glm::ivec3 get3DIndices(int, int);

        /* Volume vars */
        GLuint volumeHandle;
        Spatial *volQuad;

        /* Data stored in voxels */
        // TODO : a fixed-size array and write over values 
        std::vector<glm::vec3> voxelData;
};

#endif