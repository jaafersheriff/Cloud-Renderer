#pragma once
#ifndef _VOXELIZE_SHADER_HPP_
#define _VOXELIZE_SHADER_HPP_

#include "Shader.hpp"

#include "Model/Texture.hpp"
#include "Volume.hpp"

class VoxelizeShader : public Shader {
    public:
        VoxelizeShader(std::string vertex, std::string fragment) :
            Shader(vertex, fragment)
        {}

        bool init(Volume *, int, int);

        /* Generate 3D volume */
        void voxelize();
        void renderMesh(glm::mat4, glm::mat4, glm::vec3);

        float steps = 1.f;

        Volume * volume;
    private:
};

#endif