#pragma once
#ifndef _DIFFUSE_SHADER_HPP_
#define _DIFFUSE_SHADER_HPP_

#include "Shader.hpp"

#include <vector>

class Mesh;
class Spatial;
class DiffuseShader : public Shader {
    public:
        DiffuseShader(std::string vertex, std::string fragment) :
            Shader(vertex, fragment)
        {}

        bool init();

        void render(Mesh *mesh, std::vector<Spatial> &, glm::vec3);
};

#endif