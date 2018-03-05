#pragma once
#ifndef _VOLUME_SHADER_HPP_
#define _VOLUME_SHADER_HPP_

#include "Shader.hpp"

#include "Model/Mesh.hpp"

class VolumeShader : public Shader {
    public:
        VolumeShader(std::string vertex, std::string fragment) :
            Shader(vertex, fragment)
        {}

        bool init(int, glm::vec2, glm::vec2, glm::vec2);

        /* Generate 3D volume */
        void voxelize(Mesh *, glm::vec3 position, glm::vec3 scale);
        void clearVolume();
        void renderMesh(Mesh *, glm::vec3 position, glm::vec3 scale, bool);

        /* Getters */
        std::vector<glm::vec3> & getVoxelData() { return voxelData; }
        glm::vec2 getXBounds() { return xBounds; }
        glm::vec2 getYBounds() { return yBounds; }
        glm::vec2 getZBounds() { return zBounds; }
        int getVolumeSize() { return volumeSize; }

        /* Setters */
        void setXBounds(glm::vec2 in);
        void setYBounds(glm::vec2 in);
        void setZBounds(glm::vec2 in);
        void setVolumeSize(int);
     private:
        void initVolume();
        void generateVolume();
        glm::vec3 reverseVoxelIndex(glm::ivec3);

        /* Volume vars */
        GLuint volumeHandle;
        bool dirtyVolume = true;
        glm::vec2 xBounds;
        glm::vec2 yBounds;
        glm::vec2 zBounds;
        int volumeSize;

        /* Data stored in voxels */
        // TODO : a fixed-size array and write over values 
        std::vector<glm::vec3> voxelData;
};

#endif