#pragma once
#ifndef _DIFFUSE_SHADER_HPP_
#define _DIFFUSE_SHADER_HPP_

#include "Shader.hpp"
#include "VolumeShader.hpp"

#include <vector>

class Mesh;
class DiffuseShader : public Shader {
    public:
        DiffuseShader(std::string vertex, std::string fragment) :
            Shader(vertex, fragment)
        {}

        bool init();

        void render(std::vector<VolumeShader::Voxel> &);

        bool drawOutline = true;
};

#endif