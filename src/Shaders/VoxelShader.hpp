#pragma once
#ifndef _VOXEL_SHADER_HPP_
#define _VOXEL_SHADER_HPP_

#include "Shader.hpp"
#include "Volume.hpp"

#include <vector>

class Mesh;
class VoxelShader : public Shader {
    public:
        VoxelShader(std::string, std::string);

        void render(std::vector<Volume::Voxel> &, glm::mat4, glm::mat4);

        bool useOutline = true;
        float alpha = 0.25f;
};

#endif