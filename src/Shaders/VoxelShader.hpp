#pragma once
#ifndef _VOXEL_SHADER_HPP_
#define _VOXEL_SHADER_HPP_

#include "Shader.hpp"
#include "VolumeShader.hpp"

#include <vector>

class Mesh;
class VoxelShader : public Shader {
    public:
        VoxelShader(std::string vertex, std::string fragment) :
            Shader(vertex, fragment)
        {}

        bool init();

        void render(std::vector<VolumeShader::Voxel> &);

        bool drawOutline = true;
};

#endif