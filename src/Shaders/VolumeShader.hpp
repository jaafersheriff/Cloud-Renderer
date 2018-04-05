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
        void clearVolume();
        void voxelize(glm::mat4, glm::mat4, glm::vec3, Mesh *);
        void renderMesh(glm::mat4, glm::mat4, glm::vec3, Mesh *, bool);

        /* Getters */
        std::vector<Spatial> & getVoxelData() { return voxelData; }

        glm::vec2 xBounds;
        glm::vec2 yBounds;
        glm::vec2 zBounds;
        int volumeSize;
        float radius;
 
     private:
        void initVolume();
        glm::ivec3 get3DIndices(int);
        glm::vec3 reverseVoxelIndex(glm::ivec3);

        /* Volume vars */
        GLuint volumeHandle;
        Spatial *volQuad;

        /* Data stored in voxels */
        // TODO : a fixed-size array and write over values 
        std::vector<Spatial> voxelData;
};

#endif