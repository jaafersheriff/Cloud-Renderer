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
        void voxelize(Mesh *, glm::vec3 position = glm::vec3(0.f), glm::vec3 scale = glm::vec3(1.f));

        std::vector<glm::vec4> & getVoxelData() { return voxelData; }

    private:
        void initVolume();

        /* Volume vars */
        GLuint volumeHandle;
        int volumeSize;
        glm::vec2 xBounds;
        glm::vec2 yBounds;
        glm::vec2 zBounds;

        /* Data stored in voxels */
        std::vector<glm::vec4> voxelData;
};

#endif